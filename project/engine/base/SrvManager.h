#pragma once
#include "DirectXBase.h"

// SRV管理
class SrvManager {
public:
	static SrvManager* GetInstance();

	// 初期化
	void Initialize(DirectXBase* directXBase);

	// 終了
	void Finalize();
	
	// 確保
	uint32_t Allocate();

	// SRVの指定番号のCPUデスクリプタハンドルを取得する
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);
	// SRVの指定番号のGPUデスクリプタハンドルを取得する
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);

	// SRV生成
	void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT format, UINT MipLevels, bool isCubeMap);
	// SRV生成（Structured Buffer用）
	void CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structuredByteStride);

	// ヒープセットコマンド
	void PreDraw();

	// SRVセットコマンド
	void SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex);

	// 最大SRV数（最大テクスチャ枚数）
	static const uint32_t kMaxSRVCount_;

	bool CheckMax() const { return useIndex_ < kMaxSRVCount_; }

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() const { return descriptorHeap_; }

	// コンストラクタに渡すための鍵
	class ConstructorKey {
	private:
		ConstructorKey() = default;
		friend class SrvManager;
	};

	// PassKeyを受け取るコンストラクタ
	explicit SrvManager(ConstructorKey) {}

private:
	static std::unique_ptr<SrvManager> instance_;

	DirectXBase* directXBase_ = nullptr;

	// SRV用のデスクリプタサイズ
	uint32_t descriptorSize_ = 0;
	// SRV用デスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_;

	// 次に使用するインデックス
	uint32_t useIndex_ = 0;

	~SrvManager() = default;
	SrvManager(SrvManager&) = delete;
	SrvManager& operator=(SrvManager&) = delete;

	// default_delete にアクセスを許可する
	friend struct std::default_delete<SrvManager>;
};

