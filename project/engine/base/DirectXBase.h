#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "WinApp.h"
#include <array>
#include <dxcapi.h>
#include <chrono>
#include <externals/DirectXTex/DirectXTex.h>
#include <cstdint>

#pragma comment(lib, "dxcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

// DirectX基盤
class DirectXBase {
public:
	// シングルトンインスタンスの取得
	static DirectXBase* GetInstance();

	// 終了
	void Finalize();

	// 初期化
	void Initialize();

	// 描画前処理
	void PreDraw();
	// 描画後処理
	void PostDraw();

	// namespace省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	// getter
	ComPtr<ID3D12Device> GetDevice() const { return device_; }
	ComPtr<ID3D12GraphicsCommandList> GetCommandList() const { return commandList_; }
	ComPtr<ID3D12CommandQueue> GetCommandQueue() const { return commandQueue_; }
	ComPtr<ID3D12CommandAllocator> GetCommandAllocator() const { return commandAllocator_; }
	ComPtr<ID3D12Fence> GetFence() const { return fence_; }
	D3D12_VIEWPORT GetViewport() const { return viewport_; }
	D3D12_RECT GetScissorRect() const { return scissorRect_; }
	HANDLE GetFenceEvent() const { return fenceEvent_; }
	D3D12_RENDER_TARGET_VIEW_DESC GetRtvDesc() const { return rtvDesc_; }
	D3D12_DEPTH_STENCIL_VIEW_DESC GetDsvDesc() const { return dsvDesc_; }
	// スワップチェーンリソースの数を取得
	size_t GetSwapChainResourcesNum() const { return swapChainResources_.size(); }
	ComPtr<ID3D12DescriptorHeap> GetDsvDescriptorHeap() { return dsvDescriptorHeap_; }
	ComPtr<IDXGISwapChain> GetSwapChain() { return swapChain_; }

	// シェーダーのコンパイル
	ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile);

	// バッファリソースの生成
	ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);

	// テクスチャデータの転送
	ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);

	// テクスチャデータの転送
	ComPtr<ID3D12Resource> UploadTextureData(const ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages);

	// デスクリプタヒープを生成する
	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	// GPU実行待ち
	void WaitForGPU();

	// コンストラクタに渡すための鍵
	class ConstructorKey {
	private:
		ConstructorKey() = default;
		friend class DirectXBase;
	};

	// PassKeyを受け取るコンストラクタ
	explicit DirectXBase(ConstructorKey) {}

private:
	// 初期化
	void DeviceInitialize();

	// コマンドの初期化
	void CommandInitialize();

	// スワップチェーンの生成
	void CreateSwapChain();

	// 各種デスクリプタヒープの生成
	void CreateDescriptorHeaps();

	// レンダーターゲットビューの初期化
	void RenderTargetViewInitialize();

	// 指定番号のCPUデスクリプタハンドルを取得する
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const ComPtr<ID3D12DescriptorHeap>& descriptorHeap,
		uint32_t descriptorSize, uint32_t index);

	// 指定番号のGPUデスクリプタハンドルを取得する
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const ComPtr<ID3D12DescriptorHeap>& descriptorHeap,
		uint32_t descriptorSize, uint32_t index);

	// フェンスの初期化
	void FenceInitialize();

	// ビューポート矩形の初期化
	void ViewportInitialize();

	// シザー矩形の初期化
	void ScissorInitialize();

	// DXCコンパイラの生成
	void CreateDxcCompiler();

	// FPS固定初期化
	void InitializeFixFPS();
	// FPS固定更新
	void UpdateFixFPS();

	// インスタンス
	static std::unique_ptr<DirectXBase> instance_;

	// DirectX12デバイス
	ComPtr<ID3D12Device> device_;
	// DXGIファクトリ
	ComPtr<IDXGIFactory7> dxgiFactory_;

	// コマンドアロケータ
	ComPtr<ID3D12CommandAllocator> commandAllocator_;
	// コマンドリスト
	ComPtr<ID3D12GraphicsCommandList> commandList_;
	// コマンドキュー
	ComPtr<ID3D12CommandQueue> commandQueue_;

	// スワップチェーン
	ComPtr<IDXGISwapChain4> swapChain_;

	// スワップチェーンデスク
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc_{};

	// DepthStencilTextureをウィンドウのサイズで作成
	ComPtr<ID3D12Resource> depthStencilResource_;

	// SwapChainからResourcesを引っ張ってくる
	std::array<ComPtr<ID3D12Resource>, 2> swapChainResources_;

	// WindowsAPI
	WinApp* winApp_ = nullptr;

	// RTV用のデスクリプタヒープ生成
	uint32_t descriptorSizeRTV_ = 0;
	// DSV用のデスクリプタヒープ生成
	uint32_t descriptorSizeDSV_ = 0;

	// RTV用のヒープ
	ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_;

	// DSV用のヒープ
	ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_;

	// rtvデスク
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_{};

	// dsvデスク
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc_{};
	
	// フェンス
	ComPtr<ID3D12Fence> fence_;

	// ビューポート
	D3D12_VIEWPORT viewport_{};

	// シザー矩形
	D3D12_RECT scissorRect_{};

	// DXCユーティリティ
	ComPtr<IDxcUtils> dxcUtils_;

	// DXCコンパイラ
	ComPtr<IDxcCompiler3> dxcCompiler_;

	// デフォルトインクルードハンドラ
	ComPtr<IDxcIncludeHandler> includeHandler_;

	// バックバッファのインデックス
	UINT backBufferIndex_ = 0;

	// RTVHandle
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2]{};

	// FenceEvent
	HANDLE fenceEvent_ = nullptr;

	// フェンス値
	uint64_t fenceVal_ = 0;

	// resourceの生成
	ComPtr<ID3D12Resource> resource_;

	// 記録時間
	std::chrono::steady_clock::time_point reference_;

	~DirectXBase() = default;
	DirectXBase(const DirectXBase&) = delete;
	const DirectXBase& operator=(const DirectXBase&) = delete;

	// default_delete にアクセスを許可する
	friend struct std::default_delete<DirectXBase>;
};

