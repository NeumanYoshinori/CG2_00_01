#include "Skinning.h"

using namespace std;
using namespace MathFunction;

void Skinning::Initialize(Model* model) {
	dxBase_ = DirectXBase::GetInstance();
	srvManager_ = SrvManager::GetInstance();

	model_ = model;
	skeleton_ = CreateSkeleton(model_->GetModelData().rootNode);
	skinCluster_ = CreateSkinCluster();
}

void Skinning::Update(Skeleton& skeleton) {
	// すべてのJointを更新。親が若いので通常ループで処理可能になっている
	for (Joint& joint : skeleton.joints) {
		joint.localMatrix = MakeAffineMatrix(joint.transform.scale, joint.transform.rotate, joint.transform.translate);
		if (joint.parent) { // 親はいれば親の行列を掛ける
			joint.skeletonSpaceMatrix = joint.localMatrix * skeleton.joints[*joint.parent].skeletonSpaceMatrix;
		}
		else { // 親がいないのでlocalMatrixとskeletonSpaceMatrixは一致する
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}
	}
}

void Skinning::Update(SkinCluster& skinCluster, const Skeleton& skeleton) {
	for (size_t jointIndex = 0; jointIndex < skeleton.joints.size(); ++jointIndex) {
		assert(jointIndex < skinCluster.inverseBindPoseMatrices.size());
		skinCluster.mappedPalette[jointIndex].skeletonSpaceMatrix =
			skinCluster.inverseBindPoseMatrices[jointIndex] * skeleton.joints[jointIndex].skeletonSpaceMatrix;
		skinCluster.mappedPalette[jointIndex].skeletonSpaceInverseTransposeMatrix =
			Transpose(Inverse(skinCluster.mappedPalette[jointIndex].skeletonSpaceMatrix));
	}
}

void Skinning::ApplyAnimation(Skeleton& skeleton, const Model::Animation& animation, float animationTime) {
	for (Skinning::Joint& joint : skeleton.joints) {
		// 対象のJointのAnimationがあれば、値の適用を行う。下記のif文はC++17から可能になった初期化付きif文。
		if (auto it = animation.nodeAnimations.find(joint.name); it != animation.nodeAnimations.end()) {
			const Model::NodeAnimation& rootNodeAnimation = (*it).second;
			joint.transform.translate = model_->CalculateValue(rootNodeAnimation.translate.keyframes, animationTime);
			joint.transform.rotate = model_->CalculateValue(rootNodeAnimation.rotate.keyframes, animationTime);
			joint.transform.scale = model_->CalculateValue(rootNodeAnimation.scale.keyframes, animationTime);
		}
	}
}

void Skinning::Draw() {
	// コマンドリストを作成
	commandList_ = dxBase_->GetCommandList();

	commandList_->IASetVertexBuffers(1, 1, &skinCluster_.influenceBufferView);
	commandList_->SetGraphicsRootDescriptorTable(7, srvManager_->GetGPUDescriptorHandle(skinCluster_.srvIndex));
}

Skinning::Skeleton Skinning::CreateSkeleton(const Model::Node& rootNode) {
	Skeleton skeleton;
	skeleton.root = CreateJoint(rootNode, {}, skeleton.joints);

	// 名前とindexのマッピングを行いアクセスしやすくなる
	for (const Joint& joint : skeleton.joints) {
		skeleton.jointMap.emplace(joint.name, joint.index);
	}

	Update(skeleton);

	return skeleton;
}


int32_t Skinning::CreateJoint(const Model::Node& node, const optional<int32_t>& parent, vector<Joint>& joints) {
	Joint joint;
	joint.name = node.name;
	joint.localMatrix = node.localMatrix;
	joint.skeletonSpaceMatrix = MakeIdentity4x4();
	joint.transform = node.transform;
	joint.index = int32_t(joints.size()); // 現在登録されている数をIndexに
	joint.parent = parent;
	joints.push_back(joint); // SkeletonのJoint列に追加
	for (const Model::Node& child : node.children) {
		// 子Jointを作成し、そのIndexを登録
		int32_t childIndex = CreateJoint(child, joint.index, joints);
		joints[joint.index].children.push_back(childIndex);
	}
	// 自身のIndexを返す
	return joint.index;
}

Skinning::SkinCluster Skinning::CreateSkinCluster() {
	SkinCluster skinCluster;

	// palette用のResourceを確保
	skinCluster.paletteResource = dxBase_->CreateBufferResource(sizeof(WellForGPU) * skeleton_.joints.size());
	WellForGPU* mappedPalette = nullptr;
	skinCluster.paletteResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedPalette));
	skinCluster.mappedPalette = { mappedPalette, skeleton_.joints.size() }; // spanを使ってアクセスするようにする
	skinCluster.srvIndex = srvManager_->Allocate();

	// palette用のsrvを作成。StructuredBufferでアクセスできるようにする。
	srvManager_->CreateSRVforStructuredBuffer(skinCluster.srvIndex, skinCluster.paletteResource.Get(), UINT(skeleton_.joints.size()), sizeof(WellForGPU));

	// Influence用のresourceを確保。頂点ごとにinfluence情報を追加できるようにする
	skinCluster.influenceResource = dxBase_->CreateBufferResource(sizeof(VertexInfluence) * model_->GetModelData().vertices.size());
	VertexInfluence* mappedInfluence = nullptr;
	skinCluster.influenceResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluence));
	memset(mappedInfluence, 0, sizeof(VertexInfluence) * model_->GetModelData().vertices.size()); // 0埋め。weightを0にしておく。
	skinCluster.mappedInfluence = { mappedInfluence, model_->GetModelData().vertices.size()};

	// influence用のVBVを作成
	skinCluster.influenceBufferView.BufferLocation = skinCluster.influenceResource->GetGPUVirtualAddress();
	skinCluster.influenceBufferView.SizeInBytes = UINT(sizeof(VertexInfluence) * model_->GetModelData().vertices.size());
	skinCluster.influenceBufferView.StrideInBytes = sizeof(VertexInfluence);

	// InverseBindPoseMatrixを格納する場所を作成して、単位行列で埋める
	skinCluster.inverseBindPoseMatrices.resize(skeleton_.joints.size());
	generate(skinCluster.inverseBindPoseMatrices.begin(), skinCluster.inverseBindPoseMatrices.end(), MakeIdentity4x4);

	for (const auto& jointWeight : model_->GetModelData().skinClusterData) { // ModelのSkinClusterの情報を解析
		auto it = skeleton_.jointMap.find(jointWeight.first); // jointWeight.firstはjoint名なのｄ、skeletonに対象となるjointが含まれているか判断
		if (it == skeleton_.jointMap.end()) { // そんな名前のjointは存在しない。なので次に回す
			continue;
		}
		// (*it).secondにはjointのindexが入っているので、外套のindexのinverseBindPoseMatrixを代入
		skinCluster.inverseBindPoseMatrices[(*it).second] = jointWeight.second.inverseBindPoseMatrix;
		for (const auto& vertexWeight : jointWeight.second.vertexWeights) {
			auto& currentInfluence = skinCluster.mappedInfluence[vertexWeight.vertexIndex]; // 該当のvertexIndexのinfluence情報を参照しておく
			for (uint32_t index = 0; index < kNumMaxInfluence; ++index) { // 空いているところに入れる
				if (currentInfluence.weights[index] == 0.0f) { // weight==0が開いている状態なので、その場所にweightとjointのindexを代入
					currentInfluence.weights[index] = vertexWeight.weight;
					currentInfluence.jointIndices[index] = (*it).second;
					break;
				}
			}
		}
	}

	return skinCluster;
}
