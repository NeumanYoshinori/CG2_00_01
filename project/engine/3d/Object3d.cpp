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
		//model_->PlayAnimation();
		transformationMatrixData_->WVP = model_->GetModelData().rootNode.localMatrix * worldViewProjectionMatrix;
		transformationMatrixData_->World = model_->GetModelData().rootNode.localMatrix * worldMatrix;
	} else {
		transformationMatrixData_->WVP = worldViewProjectionMatrix;
		transformationMatrixData_->World = worldMatrix;
	}

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
	bool flipX = transformationMatrixData_->flipX;
	if (ImGui::Checkbox("FlipX", &flipX)) {
		transformationMatrixData_->flipX = flipX;
	}
	bool flipY = transformationMatrixData_->flipY;
	if (ImGui::Checkbox("FlipY", &flipY)) {
		transformationMatrixData_->flipY = flipY;
	}
#endif
}

void Object3d::SetModel(const std::string& filePath) {
	if (isModelSet_) {
		return;
	}

	// モデルを検索
	model_ = ModelManager::GetInstance()->FindModel(filePath);

	isModelSet_ = true;
}

void Object3d::SetPrimitive(const std::string& name) {
	if (isModelSet_) {
		return;
	}

	primitive_ = ModelManager::GetInstance()->FindPrimitive(name);

	isModelSet_ = true;
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
	transformationMatrixData_->flipX = false;
	transformationMatrixData_->flipY = false;
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
