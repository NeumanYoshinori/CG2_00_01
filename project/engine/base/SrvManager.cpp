#include "SrvManager.h"

const uint32_t SrvManager::kMaxSRVCount_ = 512;

using namespace Microsoft::WRL;
using namespace std;

unique_ptr<SrvManager> SrvManager::instance_ = nullptr;

SrvManager* SrvManager::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = make_unique<SrvManager>(ConstructorKey());
	}
	return instance_.get();
}

void SrvManager::Initialize() {
	// シングルトンインスタンスの取得
	directXBase_ = DirectXBase::GetInstance();

	// デスクリプタヒープの生成
	descriptorHeap_ = directXBase_->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSRVCount_, true);
	// デスクリプタ1個分のサイズを取得して記録する
	descriptorSize_ = directXBase_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void SrvManager::Finalize() {
	instance_.reset();
}

uint32_t SrvManager::Allocate() {
	// 上限に達していないかチェックしてassert
	assert(CheckMax());

	// returnする番号キーを記録しておく
	int index = useIndex_;
	// 次回のために番号を1進める
	useIndex_++;
	// 上で記録した番号をreturn
	return index;
}

D3D12_CPU_DESCRIPTOR_HANDLE SrvManager::GetCPUDescriptorHandle(uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize_ * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE SrvManager::GetGPUDescriptorHandle(uint32_t index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap_->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize_ * index);
	return handleGPU;
}

void SrvManager::CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT format, UINT MipLevels, bool isCubeMap) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	// ↑今までと一緒。metadataからcubemapかどうかを取得できるので利用して分岐
	if (isCubeMap) {
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MostDetailedMip = 0; // unionがTextureCubeになったが、内部パラメータの意味はTexture2dと変わらない
		srvDesc.TextureCube.MipLevels = UINT_MAX;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	}
	else {
		// 今まで通り2d textureの設定を行う
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
		srvDesc.Texture2D.MipLevels = MipLevels;
	}

	// 設定をもとにSRVの生成
	directXBase_->GetDevice()->CreateShaderResourceView(pResource, &srvDesc, GetCPUDescriptorHandle(srvIndex));
}

void SrvManager::CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structuredByteStride) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = numElements;
	srvDesc.Buffer.StructureByteStride = structuredByteStride;

	// 設定をもとにSRVの生成
	directXBase_->GetDevice()->CreateShaderResourceView(pResource, &srvDesc, GetCPUDescriptorHandle(srvIndex));
}

void SrvManager::PreDraw() {
	ComPtr<ID3D12DescriptorHeap> descriptorHeaps[] = { descriptorHeap_.Get() };
	directXBase_->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());
}
