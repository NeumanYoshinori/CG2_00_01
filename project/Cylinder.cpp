#include "Cylinder.h"
#include <numbers>
#include "TextureManager.h"

void Cylinder::Draw() {
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
	commandList->DrawIndexedInstanced(kNumIndex_ * kDivide_, numInstance_, 0, 0, 0);
}

void Cylinder::CreateVertexData() {
	const float kTopRadius = 1.0f;
	const float kBottomRadius = 1.0f;
	const float kHeight = 3.0f;
	const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(kDivide_);

	// 頂点リソースを作る
	vertexResource_ = dxBase_->CreateBufferResource(sizeof(VertexData) * kDivide_ * kNumVertex_);

	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * kDivide_ * kNumVertex_);
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

	for (uint32_t index = 0; index < kDivide_; ++index) {
		float sin = std::sin(index * radianPerDivide);
		float cos = std::cos(index * radianPerDivide);
		float sinNext = std::sin((index + 1) * radianPerDivide);
		float cosNext = std::cos((index + 1) * radianPerDivide);
		float u = float(index) / float(kDivide_);
		float uNext = float(index + 1) / float(kDivide_);
		// position, texcoord, normal
		vertexData_[index * kNumVertex_ + 0] = { {-sin * kTopRadius, kHeight, cos * kTopRadius, 1.0f}, {u, 0.0f}, {-sin, 0.0f, cos } };
		vertexData_[index * kNumVertex_ + 1] = { {-sinNext * kTopRadius, kHeight, cosNext * kTopRadius, 1.0f}, {uNext, 0.0f}, {-sinNext, 0.0f, cosNext} };
		vertexData_[index * kNumVertex_ + 2] = { {-sin * kBottomRadius, 0.0f, cos * kBottomRadius, 1.0f}, {u, 1.0f}, {-sin, 0.0f, cos} };
		vertexData_[index * kNumVertex_ + 3] = { {-sinNext * kBottomRadius, 0.0f, cosNext * kBottomRadius, 1.0f}, {uNext, 1.0f}, {-sinNext, 0.0f, cosNext} };
	}

	indexResource_ = dxBase_->CreateBufferResource(sizeof(uint32_t) * kDivide_ * kNumIndex_);
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));

	for (uint32_t index = 0; index < kDivide_; index++) {
		indexData_[index * kNumIndex_ + 0] = index * kNumVertex_;
		indexData_[index * kNumIndex_ + 1] = index * kNumVertex_ + 1;
		indexData_[index * kNumIndex_ + 2] = index * kNumVertex_ + 2;
		indexData_[index * kNumIndex_ + 3] = index * kNumVertex_ + 1;
		indexData_[index * kNumIndex_ + 4] = index * kNumVertex_ + 3;
		indexData_[index * kNumIndex_ + 5] = index * kNumVertex_ + 2;
	}

	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * kDivide_ * kNumIndex_;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
}
