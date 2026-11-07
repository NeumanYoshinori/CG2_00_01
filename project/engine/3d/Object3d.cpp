#include "Object3d.h"
#include "Object3dCommon.h"
#include "TextureManager.h"
#include "ModelManager.h"
#ifdef USE_IMGUI
#include <externals/imgui/imgui.h>
#endif

using namespace std;
using namespace MathFunction;

void Object3d::Initialize() {
	dxBase_ = DirectXBase::GetInstance();

	// 座標変換行列データ作成
	CreateTransformationMatrixData();

	// Transform変数を作る
	transform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

	// デフォルトカメラをセットする
	camera_ = Object3dCommon::GetInstance()->GetDefaultCamera();

	// カメラデータ作成
	CreateCameraData();

	// 環境マップデータ作成
	CreateEnvironmentMapData();
}

void Object3d::Update() {
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	Matrix4x4 worldViewProjectionMatrix;
	if (camera_) {
		const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
		worldViewProjectionMatrix = worldMatrix * viewProjectionMatrix;
	} else {
		worldViewProjectionMatrix = worldMatrix;
	}

	if (model_) {
		if (!animation_.nodeAnimations.empty()) {
			// アニメーションの時間を進める（設定する）
			animationTime_ += 1.0f / 60.0f;
			animationTime_ = fmod(animationTime_, animation_.duration);
			if (skinning_) {
				// Animationの更新を行って、骨ごとのLocal情報を更新する
				skinning_->ApplyAnimation(skeleton_, animation_, animationTime_);
				// 現在の骨ごとのLocal乗っ方を基にSkeletonSpaceの情報を更新する
				skinning_->Update(skeleton_);
				// SkeletonSpaceの情報を基に、SKinClusterのMatrixPaletteを更新する
				skinning_->Update(skinCluster_, skeleton_);
			}
			else {
				Model::NodeAnimation& rootNodeAnimation = animation_.nodeAnimations[model_->GetModelData().rootNode.name];
				Vector3 translate = model_->CalculateValue(rootNodeAnimation.translate.keyframes, animationTime_);
				Quaternion rotate = model_->CalculateValue(rootNodeAnimation.rotate.keyframes, animationTime_);
				Vector3 scale = model_->CalculateValue(rootNodeAnimation.scale.keyframes, animationTime_);
				model_->SetLocalMatrix(MakeAffineMatrix(scale, rotate, translate));
			}
		}

		if (!hasSkeleton_) {
			worldViewProjectionMatrix = model_->GetModelData().rootNode.localMatrix * worldViewProjectionMatrix;
			worldMatrix = model_->GetModelData().rootNode.localMatrix * worldMatrix;
		}
	}

	transformationMatrixData_->WVP = worldViewProjectionMatrix;
	transformationMatrixData_->World = worldMatrix;

	Matrix4x4 worldInverseMatrix = Inverse(worldMatrix);
	transformationMatrixData_->WorldInverseTranspose = Transpose(worldInverseMatrix);
}

void Object3d::Draw() {
	// コマンドリストを作成
	commandList_ = dxBase_->GetCommandList();

	// wvp用のCBufferの場所を設定
	commandList_->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());

	// ライトのcbufferの場所を設定
	lightManager_->Draw();

	// カメラのCBufferの場所を設定
	commandList_->SetGraphicsRootConstantBufferView(4, cameraResource_->GetGPUVirtualAddress());

	// SRVのDescriptorTableの先頭を設定。5はrootParameter[5]である。
	commandList_->SetGraphicsRootDescriptorTable(5, TextureManager::GetInstance()->GetSrvHandleGPU(environmentMapFilePath_));

	// 環境マップのCBufferの場所を設定
	commandList_->SetGraphicsRootConstantBufferView(6, environmentMapResource_->GetGPUVirtualAddress());

	// skinClusterの描画
	if (skinning_) {
		skinning_->Draw();
	}

	// 3Dモデルが割り当てられていれば描画する
	if (model_) {
		model_->Draw();
	}

	// プリミティブが割り当てられていれば描画する
	if (primitive_) {
		primitive_->Draw();
	}
}

void Object3d::DebugUpdate() {
#ifdef USE_IMGUI
	ImGui::DragFloat3("Scale", &transform_.scale.x, 0.01f);
	ImGui::SliderAngle("RotateX", &transform_.rotate.x);
	ImGui::SliderAngle("RotateY", &transform_.rotate.y);
	ImGui::SliderAngle("RotateZ", &transform_.rotate.z);
	ImGui::DragFloat3("Translate", &transform_.translate.x, 0.01f);
	if (model_) {
		float environmentCoeffcient = model_->GetEnvironmentCoefficient();
		ImGui::DragFloat("EnvironmentCoefficient", &environmentCoeffcient, 0.01f);
		model_->SetEnvironmentCoefficient(environmentCoeffcient);
	}
	if (primitive_) {
		float environmentCoeffcient = primitive_->GetEnvironmentCoefficient();
		ImGui::DragFloat("EnvironmentCoefficient", &environmentCoeffcient, 0.01f);
		primitive_->SetEnvironmentCoefficient(environmentCoeffcient);
	}
#endif
}

void Object3d::SetModel(const std::string& filePath) {
	if (isModelSet_) {
		return;
	}

	// モデルを検索
	model_ = ModelManager::GetInstance()->FindModel(filePath);

	if (model_) {
		animation_ = model_->GetAnimation();

		if (model_->GetModelData().rootNode.children.size() > 1) {
			skinning_ = make_unique<Skinning>();
			skinning_->Initialize(model_);
			skeleton_ = skinning_->GetSkeleton();
			skinCluster_ = skinning_->GetSkinCluster();

			hasSkeleton_ = true;
		}

		isModelSet_ = true;
	}
}

void Object3d::SetPrimitive(const std::string& name) {
	if (isModelSet_) {
		return;
	}

	primitive_ = ModelManager::GetInstance()->FindPrimitive(name);

	if (primitive_) {
		isModelSet_ = true;
	}
}

void Object3d::CreateTransformationMatrixData() {
	// TransformationMatrix用のリソースを作る。
	transformationMatrixResource_ = dxBase_->CreateBufferResource(sizeof(TransformationMatrix));

	// 書き込むためのアドレスを取得
	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));

	// 単位行列を書き込んでおく
	transformationMatrixData_->WVP = MakeIdentity4x4();
	transformationMatrixData_->World = MakeIdentity4x4();
	transformationMatrixData_->WorldInverseTranspose = MakeIdentity4x4();
}

void Object3d::CreateCameraData() {
	// カメラリソースを作る
	cameraResource_ = dxBase_->CreateBufferResource(sizeof(CameraForGPU));

	// 書き込むためのアドレスを作る
	cameraResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraData_));

	if (camera_) {
		cameraData_->worldPosition = camera_->GetTranslate();
	}
}

void Object3d::CreateEnvironmentMapData() {
	// 環境マップリソースを作る
	environmentMapResource_ = dxBase_->CreateBufferResource(sizeof(EnvironmentMap));

	// 書き込むためのアドレスを作る
	environmentMapResource_->Map(0, nullptr, reinterpret_cast<void**>(&environmentMapData_));

	// falseにしておく
	environmentMapData_->useEnvironmentMap = false;
}
