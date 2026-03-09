#include "Sprite.h"
#include "SpriteCommon.h"
#include "TextureManager.h"

using namespace Microsoft::WRL;
using namespace MathFunction;
using namespace DirectX;
using namespace std;

void Sprite::Initialize(SpriteCommon* spriteCommon, string textureFilePath) {
	// 引数で受け取ってメンバ変数に記録する
	spriteCommon_ = spriteCommon;

	dxBase_ = spriteCommon_->GetDxBase();

	// 頂点データ作成
	CreateVertexData();

	// マテリアルデータ作成
	CreateMaterialData();

	// 座標交換行列作成
	CreateTransformationMatrixData();

	// テクスチャ番号の検索と記録
	textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);

	filePath = textureFilePath;
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
	TextureManager::GetInstance()->GetMetaData(filePath);
	float tex_left = textureLeftTop.x / metadata.width;
	float tex_right = (textureLeftTop.x + textureSize.x) / metadata.width;
	float tex_top = textureLeftTop.y / metadata.height;
	float tex_bottom = (textureLeftTop.y + textureSize.y) / metadata.height;

	// 頂点リソースにデータを書き込む
	// 左下
	vertexData[0].position = { left, bottom, 0.0f, 1.0f };
	vertexData[0].texcoord = { tex_left , tex_bottom };
	vertexData[0].normal = { 0.0f, 0.0f, -1.0f };
	// 左上
	vertexData[1].position = { left, top, 0.0f, 1.0f };
	vertexData[1].texcoord = { tex_left, tex_top };
	vertexData[1].normal = { 0.0f, 0.0f, -1.0f };
	// 右下
	vertexData[2].position = { right, bottom, 0.0f, 1.0f };
	vertexData[2].texcoord = { tex_right, tex_bottom };
	vertexData[2].normal = { 0.0f, 0.0f, -1.0f };
	// 右上
	vertexData[3].position = { right, top, 0.0f, 1.0f };
	vertexData[3].texcoord = { tex_right, tex_top };
	vertexData[3].normal = { 0.0f, 0.0f, -1.0f };

	indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
	indexData[3] = 1; indexData[4] = 3; indexData[5] = 2;

	// Transform情報を作る
	transform.scale = { size.x, size.y, 1.0f };
	transform.rotate = { 0.0f, 0.0f, rotation };
	transform.translate = { position.x, position.y, 0.0f };
	// TransformからWorldMatrixを作る
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	// ViewMatrixを作って単位行列を書き込む
	Matrix4x4 viewMatrix = MakeIdentity4x4();
	// ProjectionMatrixを作って平行投影行列を書き込む
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.0f, 100.0f);
	transformationMatrixData->WVP = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	transformationMatrixData->World = worldMatrix;
}

void Sprite::Draw() {
	// コマンドリストを作成
	commandList = dxBase_->GetCommandList();

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView); // VBVを設定
	commandList->IASetIndexBuffer(&indexBufferView); // IBVを設定

	// マテリアルCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	// TransformationMatrixCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());

	// SRVのDescriptorTableの先頭を設定
	commandList->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(filePath));

	// 描画！（DrawCall/ドローコール）
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void Sprite::CreateVertexData() {
	// Sprite用の頂点リソースを作る
	vertexResource = dxBase_->CreateBufferResource(sizeof(VertexData) * 6);

	indexResource = dxBase_->CreateBufferResource(sizeof(uint32_t) * 6);

	// Sprite用の頂点リソースを作る
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 頂点バッファビューを作成する
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 6;
	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// リソースの先頭のアドレスから使う
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
	// インデックスはuint32_tとする
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	// インデックスリソースを書き込む
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
}

void Sprite::CreateMaterialData() {
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	materialResource = dxBase_->CreateBufferResource(sizeof(Material));

	// マテリアルにデータを書き込む
	// 書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

	// マテリアルデータの初期値を書き込む
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->enableLighting = false;
	materialData->uvTransform = MakeIdentity4x4();
}

void Sprite::CreateTransformationMatrixData() {
	// Sprite用のTransformationMatrix用のリソースを作る。
	transformationMatrixResource = dxBase_->CreateBufferResource(sizeof(TransformationMatrix));

	// 書き込むためのアドレスを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));

	// 単位行列を書き込んでおく
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();
}

void Sprite::AdjustTextureSize() {
	// テクスチャメタデータを取得
	const TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(filePath);

	textureSize.x = static_cast<float>(metadata.width);
	textureSize.y = static_cast<float>(metadata.height);
	// 画像サイズをテクスチャに合わせる
	size = textureSize;
}
