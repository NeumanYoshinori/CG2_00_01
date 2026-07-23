#include "Skybox.h"
#include "SkyboxCommon.h"
#include "TextureManager.h"

using namespace std;
using namespace MathFunction;

void Skybox::Initialize(string textureFilePath) {
	dxBase_ = DirectXBase::GetInstance();

	// 頂点データ作成
	CreateVertexData();

	// マテリアルデータ作成
	CreateMaterialData();

	// 座標変換行列データ作成
	CreateTransformationMatrixData();

	// Transform変数を作る
	transform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

	// デフォルトカメラをセットする
	camera_ = SkyboxCommon::GetInstance()->GetDefaultCamera();

	filePath_ = textureFilePath;
}

void Skybox::Update() {
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

void Skybox::Draw() {
	// コマンドリストを作成
	commandList_ = dxBase_->GetCommandList();

	// VertexBufferViewを設定
	commandList_->IASetVertexBuffers(0, 1, &vertexBufferView_);
	// IndexBufferViewを設定
	commandList_->IASetIndexBuffer(&indexBufferView_);
	// マテリアルCBufferの場所を設定
	commandList_->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	// wvp用のCBufferの場所を設定
	commandList_->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭を設定
	commandList_->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(filePath_));
	commandList_->DrawIndexedInstanced(kNumIndex_, 1, 0, 0, 0);
}

void Skybox::CreateVertexData() {
	// 頂点リソースを作る
	vertexResource_ = dxBase_->CreateBufferResource(sizeof(VertexData) * kNumVertex_);

	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * kNumVertex_);
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

	// 右面。描画インデックスは[0,1,2][2,1,3]で内側を向く
	vertexData_[0].position = { 1.0f, 1.0f, 1.0f, 1.0f };
	vertexData_[1].position = { 1.0f, 1.0f, -1.0f, 1.0f };
	vertexData_[2].position = { 1.0f, -1.0f, 1.0f, 1.0f };
	vertexData_[3].position = { 1.0f, -1.0f, -1.0f, 1.0f };
	// 左面。描画インデックスは[4,5,6][6,5,7]で内側を向く
	vertexData_[4].position = { -1.0f, 1.0f, -1.0f, 1.0f };
	vertexData_[5].position = { -1.0f, 1.0f, 1.0f, 1.0f };
	vertexData_[6].position = { -1.0f, -1.0f, -1.0f, 1.0f };
	vertexData_[7].position = { -1.0f, -1.0f, 1.0f, 1.0f };
	// 前面。描画インデックスは[8,9,10][10,9,11]
	vertexData_[8].position = { -1.0f, 1.0f, 1.0f, 1.0f };
	vertexData_[9].position = { 1.0f, 1.0f, 1.0f, 1.0f };
	vertexData_[10].position = { -1.0f, -1.0f, 1.0f, 1.0f };
	vertexData_[11].position = { 1.0f, -1.0f, 1.0f, 1.0f };
	// 後面。描画インデックスは[12,13,14][14,13,15]
	vertexData_[12].position = { 1.0f, 1.0f, -1.0f, 1.0f };
	vertexData_[13].position = { -1.0f, 1.0f, -1.0f, 1.0f };
	vertexData_[14].position = { 1.0f, -1.0f, -1.0f, 1.0f };
	vertexData_[15].position = { -1.0f, -1.0f, -1.0f, 1.0f };
	// 上面。描画インデックスは[16,17,18][18,17,19]
	vertexData_[16].position = { -1.0f, 1.0f, -1.0f, 1.0f };
	vertexData_[17].position = { 1.0f, 1.0f, -1.0f, 1.0f };
	vertexData_[18].position = { -1.0f, 1.0f, 1.0f, 1.0f };
	vertexData_[19].position = { 1.0f, 1.0f, 1.0f, 1.0f };
	// 下面。描画インデックスは[20,21,22][22,21,23]
	vertexData_[20].position = { -1.0f, -1.0f, 1.0f, 1.0f };
	vertexData_[21].position = { 1.0f, -1.0f, 1.0f, 1.0f };
	vertexData_[22].position = { -1.0f, -1.0f, -1.0f, 1.0f };
	vertexData_[23].position = { 1.0f, -1.0f, -1.0f, 1.0f };

	// インデックスリソースを作る
	indexResource_ = dxBase_->CreateBufferResource(sizeof(uint32_t) * kNumIndex_);
	// 書き込むためのアドレスを取得
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));

	for (uint32_t i = 0; i < kNumFace_; ++i) {
		indexData_[i * kNumFace_ + 0] = i * 4;
		indexData_[i * kNumFace_ + 1] = i * 4 + 1;
		indexData_[i * kNumFace_ + 2] = i * 4 + 2;
		indexData_[i * kNumFace_ + 3] = i * 4 + 2;
		indexData_[i * kNumFace_ + 4] = i * 4 + 1;
		indexData_[i * kNumFace_ + 5] = i * 4 + 3;
	}

	// リソースの先頭のアドレスから使う
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * kNumIndex_;
	// インデックスはuint32_tとする
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
}

void Skybox::CreateMaterialData() {
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
}

void Skybox::CreateTransformationMatrixData() {
	// TransformationMatrix用のリソースを作る。
	transformationMatrixResource_ = dxBase_->CreateBufferResource(sizeof(TransformationMatrix));

	// 書き込むためのアドレスを取得
	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));

	// 単位行列を書き込んでおく
	transformationMatrixData_->WVP = MakeIdentity4x4();
	transformationMatrixData_->World = MakeIdentity4x4();
	transformationMatrixData_->WorldInverseTranspose = MakeIdentity4x4();
}
