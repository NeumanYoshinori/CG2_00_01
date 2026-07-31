#include "Primitive.h"

void Primitive::Initialize(std::string textureFilePath, uint32_t numInstance) {
	dxBase_ = DirectXBase::GetInstance();

	// 頂点データ作成
	CreateVertexData();

	// マテリアルデータ作成
	CreateMaterialData();

	// テクスチャファイル読み込み
	textureFilePath_ = textureFilePath;

	numInstance_ = numInstance;
}

void Primitive::CreateMaterialData() {
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	materialResource_ = dxBase_->CreateBufferResource(sizeof(Material));

	// マテリアルにデータを書き込む
	// 書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	// マテリアルデータの初期値を書き込む
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_->enableLighting = false;
	materialData_->uvTransform = MathFunction::MakeIdentity4x4();
	materialData_->shininess = 10.0f;
	materialData_->environmentCoefficient = 0.0f;
	materialData_->alphaReference = 0.0f;
}
