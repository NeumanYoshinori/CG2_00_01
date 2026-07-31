#include "Sprite.h"
#include "TextureManager.h"

using namespace Microsoft::WRL;
using namespace MathFunction;
using namespace DirectX;
using namespace std;

void Sprite::Initialize(string textureFilePath) {
	dxBase_ = DirectXBase::GetInstance();

	// 頂点データ作成
	CreateVertexData();

	// マテリアルデータ作成
	CreateMaterialData();

	// 座標交換行列作成
	CreateTransformationMatrixData();

	filePath_ = textureFilePath;
	AdjustTextureSize();
}

void Sprite::Update() {
	float left = 0.0f - anchorPoint_.x;
	float right = 1.0f - anchorPoint_.x;
	float top = 0.0f - anchorPoint_.y;
	float bottom = 1.0f - anchorPoint_.y;

	// 座右反転
	if (isFlipX_) {
		left = -left;
		right = -right;
	}
	// 上下反転
	if (isFlipY_) {
		top = -top;
		bottom = -bottom;
	}

	const TexMetadata& metadata =
	TextureManager::GetInstance()->GetMetaData(filePath_);
	float tex_left = textureLeftTop_.x / metadata.width;
	float tex_right = (textureLeftTop_.x + textureSize_.x) / metadata.width;
	float tex_top = textureLeftTop_.y / metadata.height;
	float tex_bottom = (textureLeftTop_.y + textureSize_.y) / metadata.height;

	// 頂点リソースにデータを書き込む
	// 左下
	vertexData_[0].position = { left, bottom, 0.0f, 1.0f };
	vertexData_[0].texcoord = { tex_left , tex_bottom };
	vertexData_[0].normal = { 0.0f, 0.0f, -1.0f };
	// 左上
	vertexData_[1].position = { left, top, 0.0f, 1.0f };
	vertexData_[1].texcoord = { tex_left, tex_top };
	vertexData_[1].normal = { 0.0f, 0.0f, -1.0f };
	// 右下
	vertexData_[2].position = { right, bottom, 0.0f, 1.0f };
	vertexData_[2].texcoord = { tex_right, tex_bottom };
	vertexData_[2].normal = { 0.0f, 0.0f, -1.0f };
	// 右上
	vertexData_[3].position = { right, top, 0.0f, 1.0f };
	vertexData_[3].texcoord = { tex_right, tex_top };
	vertexData_[3].normal = { 0.0f, 0.0f, -1.0f };

	indexData_[0] = 0; indexData_[1] = 1; indexData_[2] = 2;
	indexData_[3] = 1; indexData_[4] = 3; indexData_[5] = 2;

	// transform情報を作る
	transform_.scale = { size_.x, size_.y, 1.0f };
	transform_.rotate = { 0.0f, 0.0f, rotation_ };
	transform_.translate = { position_.x, position_.y, 0.0f };
	// transformからWorldMatrixを作る
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	// ViewMatrixを作って単位行列を書き込む
	Matrix4x4 viewMatrix = MakeIdentity4x4();
	// ProjectionMatrixを作って平行投影行列を書き込む
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.0f, 100.0f);
	transformationMatrixData_->WVP = worldMatrix * viewMatrix * projectionMatrix;
	transformationMatrixData_->World = worldMatrix;
}

void Sprite::Draw() {
	// コマンドリストを作成
	commandList_ = dxBase_->GetCommandList();

	commandList_->IASetVertexBuffers(0, 1, &vertexBufferView_); // VBVを設定
	commandList_->IASetIndexBuffer(&indexBufferView_); // IBVを設定

	// マテリアルCBufferの場所を設定
	commandList_->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	// transformationMatrixCBufferの場所を設定
	commandList_->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());

	// SRVのDescriptorTableの先頭を設定
	commandList_->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(filePath_));

	// 描画！（DrawCall/ドローコール）
	commandList_->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void Sprite::CreateVertexData() {
	// Sprite用の頂点リソースを作る
	vertexResource_ = dxBase_->CreateBufferResource(sizeof(VertexData) * 6);

	indexResource_ = dxBase_->CreateBufferResource(sizeof(uint32_t) * 6);

	// Sprite用の頂点リソースを作る
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 頂点バッファビューを作成する
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 6;
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// リソースの先頭のアドレスから使う
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	// インデックスはuint32_tとする
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	// インデックスリソースを書き込む
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));
}

void Sprite::CreateMaterialData() {
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	materialResource_ = dxBase_->CreateBufferResource(sizeof(Material));

	// マテリアルにデータを書き込む
	// 書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	// マテリアルデータの初期値を書き込む
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_->uvTransform = MakeIdentity4x4();
}

void Sprite::CreateTransformationMatrixData() {
	// Sprite用のtransformationMatrix用のリソースを作る。
	transformationMatrixResource_ = dxBase_->CreateBufferResource(sizeof(TransformationMatrix));

	// 書き込むためのアドレスを取得
	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));

	// 単位行列を書き込んでおく
	transformationMatrixData_->WVP = MakeIdentity4x4();
	transformationMatrixData_->World = MakeIdentity4x4();
}

void Sprite::AdjustTextureSize() {
	// テクスチャメタデータを取得
	const TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(filePath_);

	textureSize_.x = static_cast<float>(metadata.width);
	textureSize_.y = static_cast<float>(metadata.height);
	// 画像サイズをテクスチャに合わせる
	size_ = textureSize_;
}
