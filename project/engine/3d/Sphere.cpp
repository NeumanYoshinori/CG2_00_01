#include "Sphere.h"
#include "Object3dCommon.h"
#include <numbers>
#include "TextureManager.h"
#include "Skybox.h"

using namespace std;
using namespace MathFunction;

void Sphere::Initialize(Object3dCommon* object3dCommon, string textureFilePath) {
	// 引数で受け取ってメンバ変数に記録する
	object3dCommon_ = object3dCommon;

	dxBase_ = object3dCommon_->GetDxBase();

	// 頂点データ作成
	CreateVertexData();

	// マテリアルデータ作成
	CreateMaterialData();

	// 座標変換行列データ作成
	CreateTransformationMatrixData();

	// Transform変数を作る
	transform = { {1.0f, 1.0f, 1.0f}, {0.0f, -1.5f, 0.0f}, {10.0f, 0.0f, 2.0f} };

	// デフォルトカメラをセットする
	camera_ = object3dCommon_->GetDefaultCamera();

	// カメラデータ作成
	CreateCameraData();

	filePath = textureFilePath;
}

void Sphere::Update() {
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 worldViewProjectionMatrix;
	if (camera_) {
		const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
		worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
	}
	else {
		worldViewProjectionMatrix = worldMatrix;
	}

	transformationMatrixData->WVP = worldViewProjectionMatrix;
	transformationMatrixData->World = worldMatrix;

	Matrix4x4 worldInverseMatrix = Inverse(worldMatrix);
	transformationMatrixData->WorldInverseTranspose = Transpose(worldInverseMatrix);
}

void Sphere::Draw() {
	// コマンドリストを作成
	commandList = dxBase_->GetCommandList();

	// VertexBufferViewを設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	// IndexVufferViewを設定
	commandList->IASetIndexBuffer(&indexBufferView);
	// マテリアルCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	// wvp用のCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭を設定
	commandList->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(filePath));
	// ライトのCBufferの場所を設定
	lightManager_->Draw();
	// カメラのCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(4, cameraResource->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭を設定
	commandList->SetGraphicsRootDescriptorTable(5, TextureManager::GetInstance()->GetSrvHandleGPU(skybox_->GetFilePath()));
	// 描画
	commandList->DrawIndexedInstanced(kSubdivision * kSubdivision * 6, 1, 0, 0, 0);
}

void Sphere::CreateVertexData() {
	// π
	float pi = std::numbers::pi_v<float>;
	const float kLonEvery = pi * 2.0f / float(kSubdivision); // 経度分割1つ分の角度
	const float kLatEvery = pi / float(kSubdivision); // 緯度分割1つ分の角度

	// 頂点リソースを作る
	vertexResource = dxBase_->CreateBufferResource(sizeof(VertexData) * (kSubdivision + 1) * (kSubdivision + 1));

	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * (kSubdivision + 1) * (kSubdivision + 1));
	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// 書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	int vertexIndex = 0;
	for (uint32_t latIndex = 0; latIndex <= kSubdivision; ++latIndex) {
		float lat = -pi / 2.0f + kLatEvery * latIndex;
		for (uint32_t lonIndex = 0; lonIndex <= kSubdivision; ++lonIndex) {
			float lon = kLonEvery * lonIndex;

			VertexData vert{
				{
					1.0f * cos(lat) * cos(lon),
					1.0f * sin(lat),
					1.0f * cos(lat) * sin(lon),
					1.0f
				},
				{
					float(lonIndex) / kSubdivision,
					1.0f - float(latIndex) / kSubdivision
				},
				{
					cos(lat) * cos(lon),
					sin(lat),
					cos(lat) * sin(lon)
				}
			};

			vertexData[vertexIndex++] = vert;
		}
	}

	indexResource = dxBase_->CreateBufferResource(sizeof(uint32_t) * kSubdivision * kSubdivision * 6);
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			uint32_t lD = lonIndex + latIndex * (kSubdivision + 1);
			uint32_t lt = lonIndex + (latIndex + 1) * (kSubdivision + 1);
			uint32_t rD = (lonIndex + 1) + latIndex * (kSubdivision + 1);
			uint32_t rt = (lonIndex + 1) + (latIndex + 1) * (kSubdivision + 1);

			uint32_t startIndex = (latIndex * kSubdivision + lonIndex) * 6;
			indexData[startIndex + 0] = lD;
			indexData[startIndex + 1] = lt;
			indexData[startIndex + 2] = rD;
			indexData[startIndex + 3] = lt;
			indexData[startIndex + 4] = rt;
			indexData[startIndex + 5] = rD;
		}
	}

	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeof(uint32_t) * kSubdivision * kSubdivision * 6;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
}

void Sphere::CreateMaterialData() {
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	materialResource = dxBase_->CreateBufferResource(sizeof(Material));

	// マテリアルにデータを書き込む
	// 書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

	// マテリアルデータの初期値を書き込む
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->enableLighting = true;
	materialData->uvTransform = MakeIdentity4x4();
	materialData->shininess = 10.0f;
	materialData->environmentCoefficient = 1.0f;
}

void Sphere::CreateTransformationMatrixData() {
	// TransformationMatrix用のリソースを作る。
	transformationMatrixResource = dxBase_->CreateBufferResource(sizeof(TransformationMatrix));

	// 書き込むためのアドレスを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));

	// 単位行列を書き込んでおく
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();
	transformationMatrixData->WorldInverseTranspose = MakeIdentity4x4();
}

void Sphere::CreateCameraData() {
	// カメラリソースを作る
	cameraResource = dxBase_->CreateBufferResource(sizeof(CameraForGPU));

	// 書き込むためのアドレスを作る
	cameraResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraData));

	if (camera_) {
		cameraData->worldPosition = camera_->GetTranslate();
	}
}
