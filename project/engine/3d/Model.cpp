#include "Model.h"
#include <cassert>
#include "TextureManager.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

using namespace std;
using namespace MathFunction;

void Model::Initialize(const string& directoryPath, const string& filename) {
	dxBase_ = DirectXBase::GetInstance();
	srvManager_ = SrvManager::GetInstance();

	// モデル読み込み
	modelData_ = LoadModelFile(directoryPath, filename);
	// アニメーション読み込み
	animation_ = LoadAnimationFile(directoryPath, filename);
	// スケルトン作成
	skeleton_ = CreateSkeleton();
	// スキンクラスター作成
	skinCluster_ = CreateSkinCluster();

	// 頂点データ作成
	CreateVertexData();

	// マテリアルデータ作成
	CreateMaterialData();

	// .objの参照しているテクスチャファイル読み込み
	TextureManager::GetInstance()->LoadTexture(modelData_.material.textureFilePath);
}

void Model::Update(Skeleton& skeleton) {
	// すべてのJointを更新。親が若いので通常ループで処理可能になっている
	for (Joint& joint : skeleton.joints) {
		joint.localMatrix = MakeAffineMatrix(joint.transform.scale, joint.transform.rotate, joint.transform.translate);
		if (joint.parent) { // 親はいれば親の行列を掛ける
			joint.skeletonSpaceMatrix = joint.localMatrix * skeleton.joints[*joint.parent].skeletonSpaceMatrix;
		} else { // 親がいないのでlocalMatrixとskeletonSpaceMatrixは一致する
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}
	}
}

void Model::Update(SkinCluster& skinCluster, const Skeleton& skeleton) {
	for (size_t jointIndex = 0; jointIndex < skeleton.joints.size(); ++jointIndex) {
		assert(jointIndex < skinCluster.inverseBindPoseMatrices.size());
		skinCluster.mappedPalette[jointIndex].skeletonSpaceMatrix =
			skinCluster.inverseBindPoseMatrices[jointIndex] * skeleton.joints[jointIndex].skeletonSpaceMatrix;
		skinCluster.mappedPalette[jointIndex].skeletonSpaceInverseTransposeMatrix =
			Transpose(Inverse(skinCluster.mappedPalette[jointIndex].skeletonSpaceMatrix));
	}
}

void Model::Draw() {
	// コマンドリストを作成
	commandList_ = dxBase_->GetCommandList();

	D3D12_VERTEX_BUFFER_VIEW vbvs[2] = {
		vertexBufferView_, // VertexDataのVBV
		skinCluster_.influenceBufferView // InfluenceのVBV
	};
	// 配列を渡す (開始slot番号、使用slot数、VBV配列へのポインタ)である。
	commandList_->IASetVertexBuffers(0, 2, vbvs);
	// IndexBufferViewを設定
	commandList_->IASetIndexBuffer(&indexBufferView_);
	// マテリアルCBufferの場所を設定
	commandList_->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
	commandList_->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(modelData_.material.textureFilePath));
	commandList_->SetGraphicsRootDescriptorTable(7, srvManager_->GetGPUDescriptorHandle(skinCluster_.srvIndex));
	if (numInstance_ == 0) {
		return;
	}
	// 描画！（フォローコール）
	commandList_->DrawIndexedInstanced(UINT(modelData_.indices.size()), numInstance_, 0, 0, 0);
}

Model::ModelData Model::LoadModelFile(const string& directoryPath, const string& filename) {
	// 中で必要となる変数の宣言
	ModelData modelData; // 構築するModelData

	Assimp::Importer importer;
	string filePath = directoryPath + "/" + filename;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
	assert(scene->HasMeshes()); // メッシュがないのは対応しない

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals()); // 法線がないMeshは今回は非対応
		assert(mesh->HasTextureCoords(0)); // TexcoordがないMeshは今回は非対応
		modelData.vertices.resize(mesh->mNumVertices); // 最初に頂点数分のメモリを確保しておく
		for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
			aiVector3D& position = mesh->mVertices[vertexIndex];
			aiVector3D& normal = mesh->mNormals[vertexIndex];
			aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
			// 右手系->左手系への変換を忘れずに
			modelData.vertices[vertexIndex].position = { -position.x, position.y, position.z, 1.0f };
			modelData.vertices[vertexIndex].normal = { -normal.x, normal.y, normal.z };
			modelData.vertices[vertexIndex].texcoord = { texcoord.x, texcoord.y };
		}
		// ここからFaceの中身(face)の解析を行っていく
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3); // 三角形のみサポート
			// ここからFaceの中身(Vertex)の解析を行っていく
			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				uint32_t vertexIndex = face.mIndices[element];
				modelData.indices.push_back(vertexIndex);
			}
		}
		for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
			aiBone* bone = mesh->mBones[boneIndex];
			string jointName = bone->mName.C_Str();
			JointWeightData& jointWeightData = modelData.skinClusterData[jointName];

			aiMatrix4x4 bindPoseMatrixAssimp = bone->mOffsetMatrix.Inverse();
			aiVector3D scale, translate;
			aiQuaternion rotate;
			bindPoseMatrixAssimp.Decompose(scale, rotate, translate);
			Matrix4x4 bindPoseMatrix = MakeAffineMatrix(
				{ scale.x, scale.y, scale.z }, { rotate.x, -rotate.y, -rotate.z, rotate.w }, { -translate.x, translate.y, translate.z });
			jointWeightData.inverseBindPoseMatrix = Inverse(bindPoseMatrix);

			for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
				jointWeightData.vertexWeights.push_back({ bone->mWeights[weightIndex].mWeight, bone->mWeights[weightIndex].mVertexId });
			}
		}
	}

	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
			modelData.material.textureFilePath = textureFilePath.C_Str();
		}
	}

	modelData.rootNode = ReadNode(scene->mRootNode);

	return modelData;
}

Model::Node Model::ReadNode(aiNode* node) {
	Node result;
	aiVector3D scale, translate;
	aiQuaternion rotate;
	node->mTransformation.Decompose(scale, rotate, translate); // assimpの行列からSRTを抽出する関数を利用
	result.transform.scale = { scale.x, scale.y, scale.z };
	result.transform.rotate = { rotate.x, -rotate.y, -rotate.z, rotate.w };
	result.transform.translate = { -translate.x, translate.y, translate.z };
	result.localMatrix = MakeAffineMatrix(result.transform.scale, result.transform.rotate, result.transform.translate);
	result.name = node->mName.C_Str(); // Node名を格納
	result.children.resize(node->mNumChildren); // 子供の数だけ確保
	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
		// 再帰的に呼んで階層構造を作っていく
		result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
	}
	return result;
}

Model::Animation Model::LoadAnimationFile(const string& directoryPath, const string& filename) {
	Animation animation; // 今回作るアニメーション
	Assimp::Importer importer;
	string filePath = directoryPath + "/" + filename;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), 0);
	if (scene->mNumAnimations == 0) { // アニメーションがない
		return animation;
	}
	aiAnimation* animationAssimp = scene->mAnimations[0]; // 最初のアニメーションだけ採用。もちろん複数対応するに越したことはない
	animation.duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond); // 時間の単位を秒に変換

	// assimpでは個々のNodeのAnimationをchannelと呼んでいるのでchanelを回してNodeAnimationの情報をとってくる
	for (uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex) {
		aiNodeAnim* nodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
		NodeAnimation& nodeAnimation = animation.nodeAnimations[nodeAnimationAssimp->mNodeName.C_Str()];
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys; ++keyIndex) {
			aiVectorKey keyAssimp = nodeAnimationAssimp->mPositionKeys[keyIndex];
			KeyframeVector3 keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond); // ここも秒に変換
			keyframe.value = { -keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z }; // 右手->左手
			nodeAnimation.translate.keyframes.push_back(keyframe);
		}

		// RotateはmNumRotationKeys/mRotationKeys、ScaleはmNumScalingKeysで取得できるので同様に行う。
		// RotateはQuaternionで、右手->左手に変換するために、yとzを反転させる必要がある。Scaleはそのままで良い。
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; ++keyIndex) {
			aiQuatKey keyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
			KeyframeQuaternion keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond); // ここも秒に変換
			keyframe.value = { keyAssimp.mValue.x, -keyAssimp.mValue.y, -keyAssimp.mValue.z, keyAssimp.mValue.w }; // 右手->左手
			nodeAnimation.rotate.keyframes.push_back(keyframe);
		}

		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumScalingKeys; ++keyIndex) {
			aiVectorKey keyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
			KeyframeVector3 keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond); // ここも秒に変換
			keyframe.value = { keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z };
			nodeAnimation.scale.keyframes.push_back(keyframe);
		}
	}
	// 解析完了
	return animation;
}

Vector3 Model::CalculateValue(const vector<KeyframeVector3>& keyframes, float time) {
	assert(!keyframes.empty()); // キーがないものは返す値がわからないのでダメ
	if (keyframes.size() == 1 || time <= keyframes[0].time) { // キーが1つか、時刻がキーフレーム前なら最初の値とする
		return keyframes[0].value;
	}

	for (size_t index = 0; index < keyframes.size() - 1; ++index) {
		size_t nextIndex = index + 1;
		// indexとnextIndexの2つのkeyframeを取得して範囲内に時刻があるかを判定
		if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
			// 範囲内を補間する
			float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
			return Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
		}
	}
	// ここまで来た場合は一番後の時刻よりも後ろなので最後の値を返すことにする
	return (*keyframes.rbegin()).value;
}

Quaternion Model::CalculateValue(const vector<KeyframeQuaternion>& keyframes, float time) {
	assert(!keyframes.empty()); // キーがないものは返す値がわからないのでダメ
	if (keyframes.size() == 1 || time <= keyframes[0].time) { // キーが1つか、時刻がキーフレーム前なら最初の値とする
		return keyframes[0].value;
	}

	for (size_t index = 0; index < keyframes.size() - 1; ++index) {
		size_t nextIndex = index + 1;
		// indexとnextIndexの2つのkeyframeを取得して範囲内に時刻があるかを判定
		if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
			// 範囲内を補間する
			float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
			return Slerp(keyframes[index].value, keyframes[nextIndex].value, t);
		}
	}
	// ここまで来た場合は一番後の時刻よりも後ろなので最後の値を返すことにする
	return (*keyframes.rbegin()).value;
}

Model::Skeleton Model::CreateSkeleton() {
	Skeleton skeleton;
	skeleton.root = CreateJoint(modelData_.rootNode, {}, skeleton.joints);

	// 名前とindexのマッピングを行いアクセスしやすくなる
	for (const Joint& joint : skeleton.joints) {
		skeleton.jointMap.emplace(joint.name, joint.index);
	}

	Update(skeleton);

	return skeleton;
}

int32_t Model::CreateJoint(const Node& node, const optional<int32_t>& parent, vector<Joint>& joints) {
	Joint joint;
	joint.name = node.name;
	joint.localMatrix = node.localMatrix;
	joint.skeletonSpaceMatrix = MakeIdentity4x4();
	joint.transform = node.transform;
	joint.index = int32_t(joints.size()); // 現在登録されている数をIndexに
	joint.parent = parent;
	joints.push_back(joint); // SkeletonのJoint列に追加
	for (const Node& child : node.children) {
		// 子Jointを作成し、そのIndexを登録
		int32_t childIndex = CreateJoint(child, joint.index, joints);
		joints[joint.index].children.push_back(childIndex);
	}
	// 自身のIndexを返す
	return joint.index;
}

void Model::ApplyAnimation(Skeleton& skeleton, const Animation& animation, float animationTime) {
	for (Joint& joint : skeleton.joints) {
		// 対象のJointのAnimationがあれば、値の適用を行う。下記のif文はC++17から可能になった初期化付きif文。
		if (auto it = animation.nodeAnimations.find(joint.name); it != animation.nodeAnimations.end()) {
			const NodeAnimation& rootNodeAnimation = (*it).second;
			joint.transform.translate = CalculateValue(rootNodeAnimation.translate.keyframes, animationTime);
			joint.transform.rotate = CalculateValue(rootNodeAnimation.rotate.keyframes, animationTime);
			joint.transform.scale = CalculateValue(rootNodeAnimation.scale.keyframes, animationTime);
		}
	}
}

Model::SkinCluster Model::CreateSkinCluster() {
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
	skinCluster.influenceResource = dxBase_->CreateBufferResource(sizeof(VertexInfluence) * modelData_.vertices.size());
	VertexInfluence* mappedInfluence = nullptr;
	skinCluster.influenceResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluence));
	memset(mappedInfluence, 0, sizeof(VertexInfluence) * modelData_.vertices.size()); // 0埋め。weightを0にしておく。
	skinCluster.mappedInfluence = { mappedInfluence, modelData_.vertices.size() };

	// influence用のVBVを作成
	skinCluster.influenceBufferView.BufferLocation = skinCluster.influenceResource->GetGPUVirtualAddress();
	skinCluster.influenceBufferView.SizeInBytes = UINT(sizeof(VertexInfluence) * modelData_.vertices.size());
	skinCluster.influenceBufferView.StrideInBytes = sizeof(VertexInfluence);

	// InverseBindPoseMatrixを格納する場所を作成して、単位行列で埋める
	skinCluster.inverseBindPoseMatrices.resize(skeleton_.joints.size());
	generate(skinCluster.inverseBindPoseMatrices.begin(), skinCluster.inverseBindPoseMatrices.end(), MakeIdentity4x4);

	for (const auto& jointWeight : modelData_.skinClusterData) { // ModelのSkinClusterの情報を解析
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

void Model::CreateVertexData() {
	// 頂点リソースを作る
	vertexResource_ = dxBase_->CreateBufferResource(sizeof(VertexData) * modelData_.vertices.size());

	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズはモデルの頂点のサイズ
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	memcpy(vertexData_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());

	// インデックスリソースを作る
	indexResource_ = dxBase_->CreateBufferResource(sizeof(uint32_t) * modelData_.indices.size());

	// リソースの先頭のアドレスから使う
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズはモデルの頂点のサイズ
	indexBufferView_.SizeInBytes = UINT(sizeof(uint32_t) * modelData_.indices.size());
	// 1頂点あたりのサイズ
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	// 書き込むためのアドレスを取得
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));
	memcpy(indexData_, modelData_.indices.data(), sizeof(uint32_t) * modelData_.indices.size());
}

void Model::CreateMaterialData() {
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	materialResource_ = dxBase_->CreateBufferResource(sizeof(Material));

	// マテリアルにデータを書き込む
	// 書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	// マテリアルデータの初期値を書き込む
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_->enableLighting = true;
	materialData_->uvTransform = MakeIdentity4x4();
	materialData_->shininess = 10.0f;
	materialData_->environmentCoefficient = 1.0f;
	materialData_->alphaReference = 0.0f;
}

