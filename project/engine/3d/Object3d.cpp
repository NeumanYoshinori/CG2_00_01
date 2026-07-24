#include "Object3d.h"
#include "Object3dCommon.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "ImGuiManager.h"

using namespace std;
using namespace MathFunction;

void Object3d::Initialize() {
	dxBase_ = DirectXBase::GetInstance();

	// 座標変換行列データ作成
	CreateTransformationMatrixData();

	// Transform変数を作る
	transform = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	
	// デフォルトカメラをセットする
	camera_ = Object3dCommon::GetInstance()->GetDefaultCamera();

	// カメラデータ作成
	CreateCameraData();
}

void Object3d::Update() {
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 worldViewProjectionMatrix;
	if (camera_) {
		const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
		worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
	} else {
		worldViewProjectionMatrix = worldMatrix;
	}

	transformationMatrixData->WVP = model_->GetModelData().rootNode.localMatrix * worldViewProjectionMatrix;
	transformationMatrixData->World = model_->GetModelData().rootNode.localMatrix * worldMatrix;

	Matrix4x4 worldInverseMatrix = Inverse(worldMatrix);
	transformationMatrixData->WorldInverseTranspose = Transpose(worldInverseMatrix);
}

void Object3d::Draw() {
	// コマンドリストを作成
	commandList = dxBase_->GetCommandList();

	// wvp用のCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());

	// ライトのcbufferの場所を設定
	lightManager_->Draw();

	// カメラのCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(4, cameraResource->GetGPUVirtualAddress());

	// SRVのDescriptorTableの先頭を設定。5はrootParameter[5]である。
	commandList->SetGraphicsRootDescriptorTable(5, TextureManager::GetInstance()->GetSrvHandleGPU(skybox_->GetFilePath()));

	// 3Dモデルが割り当てられていれば描画する
	if (model_) {
		model_->Draw();
	}
}

void Object3d::DebugUpdate() {
#ifdef USE_IMGUI
	ImGui::DragFloat3("Scale", &transform.scale.x, 0.01f);
	ImGui::SliderAngle("RotateX", &transform.rotate.x);
	ImGui::SliderAngle("RotateY", &transform.rotate.y);
	ImGui::SliderAngle("RotateZ", &transform.rotate.z);
	ImGui::DragFloat3("Translate", &transform.translate.x, 0.01f);
	float environmentCoeffcient = model_->GetEnvironmentCoefficient();
	ImGui::DragFloat("EnvironmentCoefficient", &environmentCoeffcient, 0.01f);
	model_->SetEnvironmentCoefficient(environmentCoeffcient);
#endif
}

void Object3d::SetModel(const std::string& filePath) {
	// モデルを検索
	model_ = ModelManager::GetInstance()->FindModel(filePath);
}

void Object3d::CreateTransformationMatrixData() {
	// TransformationMatrix用のリソースを作る。
	transformationMatrixResource = dxBase_->CreateBufferResource(sizeof(TransformationMatrix));

	// 書き込むためのアドレスを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));

	// 単位行列を書き込んでおく
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();
	transformationMatrixData->WorldInverseTranspose = MakeIdentity4x4();
}

void Object3d::CreateCameraData() {
	// カメラリソースを作る
	cameraResource = dxBase_->CreateBufferResource(sizeof(CameraForGPU));

	// 書き込むためのアドレスを作る
	cameraResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraData));

	if (camera_) {
		cameraData->worldPosition = camera_->GetTranslate();
	}
}
