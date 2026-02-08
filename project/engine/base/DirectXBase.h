#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "WinApp.h"
#include <array>
#include <dxcapi.h>
#include <chrono>
#include <externals/DirectXTex/DirectXTex.h>
#include <externals/DirectXTex/d3dx12.h>
#include "Logger.h"
#include "StringUtility.h"
#include <cstdint>

#pragma comment(lib, "dxcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

// DirectX基盤
class DirectXBase {
public:
	// 初期化
	void Initialize(WinApp* winApp);

	// 描画前処理
	void PreDraw();
	// 描画後処理
	void PostDraw();

	// getter
	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() const { return device; }
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() const { return commandList; }
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() const { return commandQueue; }
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> GetCommandAllocator() const { return commandAllocator; }
	Microsoft::WRL::ComPtr<ID3D12Fence> GetFence() const { return fence; }
	D3D12_VIEWPORT GetViewport() const { return viewport; }
	D3D12_RECT GetScissorRect() const { return scissorRect; }
	HANDLE GetFenceEvent() const { return fenceEvent; }

	// シェーダーのコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile);

	// バッファリソースの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);

	// テクスチャデータの転送
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);

	// テクスチャデータの転送
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(const Microsoft::WRL::ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages);

	// FPS固定初期化
	void InitializeFixFPS();
	// FPS固定更新
	void UpdateFixFPS();

	// デスクリプタヒープを生成する
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

private:
	// 初期化
	void DeviceInitialize();

	// コマンドの初期化
	void CommandInitialize();

	// スワップチェーンの生成
	void CreateSwapChain();

	// 深度バッファの生成
	void CreateDepthBuffer();

	// 各種デスクリプタヒープの生成
	void CreateDescriptorHeaps();

	// レンダーターゲットビューの初期化
	void RenderTargetViewInitialize();

	// 指定番号のCPUデスクリプタハンドルを取得する
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap,
		uint32_t descriptorSize, uint32_t index);

	// 指定番号のGPUデスクリプタハンドルを取得する
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap,
		uint32_t descriptorSize, uint32_t index);

	// 深度ステンシルビューの初期化
	void DepthStencilInitialize();

	// フェンスの初期化
	void FenceInitialize();

	// ビューポート矩形の初期化
	void ViewportInitialize();

	// シザー矩形の初期化
	void ScissorInitalize();

	// DXCコンパイラの生成
	void CreateDxcCompiler();

	// ImGuiの初期化
	//void ImGuiInitialize();

	// DirectX12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;
	// DXGIファクトリ
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;

	// コマンドアロケータ
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	// コマンドキュー
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;

	// スワップチェーン
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;

	// スワップチェーンデスク
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};

	// DepthStencilTextureをウィンドウのサイズで作成
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource;

	// SwapChainからResourcesを引っ張ってくる
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;

	// WindowsAPI
	WinApp* winApp_ = nullptr;

	// RTV用のデスクリプタヒープ生成
	uint32_t descriptorSizeRTV = 0;
	// DSV用のデスクリプタヒープ生成
	uint32_t descriptorSizeDSV = 0;

	// RTV用のヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = nullptr;

	// DSV用のヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = nullptr;

	// rtvデスク
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	
	// フェンス
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;

	// ビューポート
	D3D12_VIEWPORT viewport{};

	// シザー矩形
	D3D12_RECT scissorRect{};

	// DXCユーティリティ
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils;

	// DXCコンパイラ
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler;

	// デフォルトインクルードハンドラ
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler;

	// バックバッファのインデックス
	UINT backBufferIndex;

	// TransitionBarrier
	D3D12_RESOURCE_BARRIER barrier{};

	// RTVHandle
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];

	// FenceEvent
	HANDLE fenceEvent = nullptr;

	// フェンス値
	uint64_t fenceVal = 0;

	// resourceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;

	// 記録時間
	std::chrono::steady_clock::time_point reference_;
};

