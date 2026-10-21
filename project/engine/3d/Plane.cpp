#include "Plane.h"
#include "TextureManager.h"

void Plane::Draw() {
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = dxBase_->GetCommandList();
	// VertexBufferViewを設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	// IndexBufferViewを設定
	commandList->IASetIndexBuffer(&indexBufferView_);
	// マテリアルCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureFilePath_));
	if (numInstance_ == 0) {
		return;
	}
	commandList->DrawIndexedInstanced(kNumIndex_, numInstance_, 0, 0, 0);
}

void Plane::CreateVertexData() {
	// 頂点リソースを作る
	vertexResource_ = dxBase_->CreateBufferResource(sizeof(VertexData) * kNumVertex_);

	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点のサイズ
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * kNumVertex_;
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

	// 左上
	vertexData_[0].position = { 1.0f, 1.0f, 0.0f, 1.0f };
	vertexData_[0].texcoord = { 0.0f, 0.0f };
	vertexData_[0].normal = { 0.0f, 0.0f, 1.0f };

	// 右上
	vertexData_[1].position = { -1.0f, 1.0f, 0.0f, 1.0f };
	vertexData_[1].texcoord = { 1.0f, 0.0f };
	vertexData_[1].normal = { 0.0f, 0.0f, 1.0f };

	// 左下
	vertexData_[2].position = { 1.0f, -1.0f, 0.0f, 1.0f };
	vertexData_[2].texcoord = { 0.0f, 1.0f };
	vertexData_[2].normal = { 0.0f, 0.0f, 1.0f };

	// 右下
	vertexData_[3].position = { -1.0f, -1.0f, 0.0f, 1.0f };
	vertexData_[3].texcoord = { 1.0f, 1.0f };
	vertexData_[3].normal = { 0.0f, 0.0f, 1.0f };

	indexResource_ = dxBase_->CreateBufferResource(sizeof(uint32_t) * kNumIndex_);
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));

	indexData_[0] = 0; indexData_[1] = 1; indexData_[2] = 2;
	indexData_[3] = 1; indexData_[4] = 3; indexData_[5] = 2;

	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * kNumIndex_;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
}
