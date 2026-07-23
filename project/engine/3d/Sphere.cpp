#include "Sphere.h"
#include "Object3dCommon.h"
#include <numbers>
#include "TextureManager.h"
#include "Skybox.h"

using namespace std;
using namespace MathFunction;

void Sphere::Initialize(string textureFilePath) {
	dxBase_ = DirectXBase::GetInstance();

	// 頂点データ作成
	CreateVertexData();

	// マテリアルデータ作成
	CreateMaterialData();

	// 座標変換行列データ作成
	CreateTransformationMatrixData();

	// Transform変数を作る
	transform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, -1.5f, 0.0f}, {10.0f, 0.0f, 2.0f} };

	// デフォルトカメラをセットする
	camera_ = Object3dCommon::GetInstance()->GetDefaultCamera();

	// カメラデータ作成
	CreateCameraData();

	// テクスチャファイル読み込み
	textureFilePath_ = textureFilePath;
}

void Sphere::Update() {
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	Matrix4x4 worldViewProjectionMatrix;
	if (camera_) {
		const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
		worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
	}
	else {
		worldViewProjectionMatrix = worldMatrix;
	}

	transformationMatrixData_->WVP = worldViewProjectionMatrix;
	transformationMatrixData_->World = worldMatrix;

	Matrix4x4 worldInverseMatrix = Inverse(worldMatrix);
	transformationMatrixData_->WorldInverseTranspose = Transpose(worldInverseMatrix);
}

void Sphere::Draw() {
	// コマンドリストを作成
	commandList_ = dxBase_->GetCommandList();

	// vertexBufferView_を設定
	commandList_->IASetVertexBuffers(0, 1, &vertexBufferView_);
	// indexBufferView_を設定
	commandList_->IASetIndexBuffer(&indexBufferView_);
	// マテリアルCBufferの場所を設定
	commandList_->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	// wvp用のCBufferの場所を設定
	commandList_->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭を設定
	commandList_->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureFilePath_));
	// ライトのCBufferの場所を設定
	lightManager_->Draw();
	// カメラのCBufferの場所を設定
	commandList_->SetGraphicsRootConstantBufferView(4, cameraResource_->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭を設定
	commandList_->SetGraphicsRootDescriptorTable(5, TextureManager::GetInstance()->GetSrvHandleGPU(skybox_->GetFilePath()));
	// 描画
	commandList_->DrawIndexedInstanced(kSubdivision_ * kSubdivision_ * 6, 1, 0, 0, 0);
}

void Sphere::CreateVertexData() {
	// π
	float pi = std::numbers::pi_v<float>;
	const float kLonEvery = pi * 2.0f / float(kSubdivision_); // 経度分割1つ分の角度
	const float kLatEvery = pi / float(kSubdivision_); // 緯度分割1つ分の角度

	// 頂点リソースを作る
	vertexResource_ = dxBase_->CreateBufferResource(sizeof(VertexData) * (kSubdivision_ + 1) * (kSubdivision_ + 1));

	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * (kSubdivision_ + 1) * (kSubdivision_ + 1));
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

	int vertexIndex = 0;
	for (uint32_t latIndex = 0; latIndex <= kSubdivision_; ++latIndex) {
		float lat = -pi / 2.0f + kLatEvery * latIndex;
		for (uint32_t lonIndex = 0; lonIndex <= kSubdivision_; ++lonIndex) {
			float lon = kLonEvery * lonIndex;

			VertexData vert{
				{
					cos(lat) * cos(lon),
					sin(lat),
					cos(lat) * sin(lon),
					1.0f
				},
				{
					float(lonIndex) / kSubdivision_,
					1.0f - float(latIndex) / kSubdivision_
				},
				{
					cos(lat) * cos(lon),
					sin(lat),
					cos(lat) * sin(lon)
				}
			};

			vertexData_[vertexIndex++] = vert;
		}
	}

	indexResource_ = dxBase_->CreateBufferResource(sizeof(uint32_t) * kSubdivision_ * kSubdivision_ * 6);
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));

	for (uint32_t latIndex = 0; latIndex < kSubdivision_; ++latIndex) {
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision_; ++lonIndex) {
			uint32_t lD = lonIndex + latIndex * (kSubdivision_ + 1);
			uint32_t lt = lonIndex + (latIndex + 1) * (kSubdivision_ + 1);
			uint32_t rD = lonIndex + 1 + latIndex * (kSubdivision_ + 1);
			uint32_t rt = lonIndex + 1 + (latIndex + 1) * (kSubdivision_ + 1);

			uint32_t startIndex = (latIndex * kSubdivision_ + lonIndex) * 6;
			indexData_[startIndex + 0] = lD;
			indexData_[startIndex + 1] = lt;
			indexData_[startIndex + 2] = rD;
			indexData_[startIndex + 3] = lt;
			indexData_[startIndex + 4] = rt;
			indexData_[startIndex + 5] = rD;
		}
	}

	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * kSubdivision_ * kSubdivision_ * 6;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
}

void Sphere::CreateMaterialData() {
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
}

void Sphere::CreateTransformationMatrixData() {
	// transformationMatrix用のリソースを作る。
	transformationMatrixResource_ = dxBase_->CreateBufferResource(sizeof(TransformationMatrix));

	// 書き込むためのアドレスを取得
	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));

	// 単位行列を書き込んでおく
	transformationMatrixData_->WVP = MakeIdentity4x4();
	transformationMatrixData_->World = MakeIdentity4x4();
	transformationMatrixData_->WorldInverseTranspose = MakeIdentity4x4();
}

void Sphere::CreateCameraData() {
	// カメラリソースを作る
	cameraResource_ = dxBase_->CreateBufferResource(sizeof(CameraForGPU));

	// 書き込むためのアドレスを作る
	cameraResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraData_));

	if (camera_) {
		cameraData_->worldPosition = camera_->GetTranslate();
	}
}
