#include "Line.h"

using namespace Microsoft::WRL;
using namespace MathFunction;

void Line::Initialize(Vector2 start, Vector2 end, Vector4 color) {
	dxBase_ = DirectXBase::GetInstance();

	start_ = start;
	end_ = end;

	color_ = color;

	// Transform変数を作る
	transform_ = { {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

	CreateVertexData();
	CreateMaterialData();
	CreateTransformationMatrixData();
}

void Line::Update() {
	vertexData_[0].position = Vector4(start_.x, start_.y, 0.0f, 1.0f);
	vertexData_[1].position = Vector4(end_.x, end_.y, 0.0f, 1.0f);

	// transformからWorldMatrixを作る
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	// ViewMatrixを作って単位行列を書き込む
	Matrix4x4 viewMatrix = MakeIdentity4x4();
	// ProjectionMatrixを作って平行投影行列を書き込む
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.0f, 100.0f);
	transformationMatrixData_->WVP = worldMatrix * viewMatrix * projectionMatrix;
	transformationMatrixData_->World = worldMatrix;
}

void Line::Draw() {
	// コマンドリストを作成
	commandList_ = dxBase_->GetCommandList();

	commandList_->IASetVertexBuffers(0, 1, &vertexBufferView_); // VBVを設定

	// マテリアルCBufferの場所を設定
	commandList_->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	// transformationMatrixCBufferの場所を設定
	commandList_->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());

	// 描画！（DrawCall/ドローコール）
	commandList_->DrawInstanced(2, 1, 0, 0);
}

void Line::CreateVertexData() {
	vertexResource_ = dxBase_->CreateBufferResource(sizeof(VertexData) * 2);

	// Sprite用の頂点リソースを作る
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 頂点バッファビューを作成する
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 2;
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
}

void Line::CreateMaterialData() {
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	materialResource_ = dxBase_->CreateBufferResource(sizeof(Material));

	// マテリアルにデータを書き込む
	// 書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	// マテリアルデータの初期値を書き込む
	materialData_->color = { color_ };
}

void Line::CreateTransformationMatrixData() {
	// Sprite用のtransformationMatrix用のリソースを作る。
	transformationMatrixResource_ = dxBase_->CreateBufferResource(sizeof(TransformationMatrix));

	// 書き込むためのアドレスを取得
	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));

	// 単位行列を書き込んでおく
	transformationMatrixData_->WVP = MakeIdentity4x4();
	transformationMatrixData_->World = MakeIdentity4x4();
}
