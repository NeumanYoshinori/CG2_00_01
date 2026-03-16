#pragma once
#include "DirectXBase.h"

// SRV管理
class SrvManager {
public:
	// 初期化
	void Initialize(DirectXBase* directXBase);
	
	// 確保
	uint32_t Allocate();

	// SRVの指定番号のCPUデスクリプタハンドルを取得する
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);
	// SRVの指定番号のGPUデスクリプタハンドルを取得する
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);

	// SRV生成
	void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT format, UINT MipLevels);
	// SRV生成（Structured Buffer用）
	void CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structuredByteStride);

	// ヒープセットコマンド
	void PreDraw();

	// SRVセットコマンド
	void SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex);

	// 最大SRV数（最大テクスチャ枚数）
	static const uint32_t kMaxSRVCount;

	bool CheckMax() const { return useIndex < kMaxSRVCount; }

private:
	DirectXBase* directXBase_ = nullptr;

	// SRV用のデスクリプタサイズ
	uint32_t descriptorSize;
	// SRV用デスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;

	// 次に使用するインデックス
	uint32_t useIndex = 0;
};

