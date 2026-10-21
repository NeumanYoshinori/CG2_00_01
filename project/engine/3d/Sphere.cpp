#include "Sphere.h"
#include <numbers>
#include "TextureManager.h"

using namespace std;
using namespace MathFunction;

void Sphere::Draw() {
	// コマンドリストを作成
	commandList_ = dxBase_->GetCommandList();

	// vertexBufferView_を設定
	commandList_->IASetVertexBuffers(0, 1, &vertexBufferView_);
	// indexBufferView_を設定
	commandList_->IASetIndexBuffer(&indexBufferView_);
	// マテリアルCBufferの場所を設定
	commandList_->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭を設定
	commandList_->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureFilePath_));
	if (numInstance_ == 0) {
		return;
	}
	// 描画
	commandList_->DrawIndexedInstanced(kSubdivision_ * kSubdivision_ * kNumIndex_, numInstance_, 0, 0, 0);
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

			VertexData vert {
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

	indexResource_ = dxBase_->CreateBufferResource(sizeof(uint32_t) * kSubdivision_ * kSubdivision_ * kNumIndex_);
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));

	for (uint32_t latIndex = 0; latIndex < kSubdivision_; ++latIndex) {
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision_; ++lonIndex) {
			uint32_t lD = lonIndex + latIndex * (kSubdivision_ + 1);
			uint32_t lt = lonIndex + (latIndex + 1) * (kSubdivision_ + 1);
			uint32_t rD = lonIndex + 1 + latIndex * (kSubdivision_ + 1);
			uint32_t rt = lonIndex + 1 + (latIndex + 1) * (kSubdivision_ + 1);

			uint32_t startIndex = (latIndex * kSubdivision_ + lonIndex) * kNumIndex_;
			indexData_[startIndex + 0] = lD;
			indexData_[startIndex + 1] = lt;
			indexData_[startIndex + 2] = rD;
			indexData_[startIndex + 3] = lt;
			indexData_[startIndex + 4] = rt;
			indexData_[startIndex + 5] = rD;
		}
	}

	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * kSubdivision_ * kSubdivision_ * kNumIndex_;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
}
