#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>
#include "WinApp.h"
#include "Vector4.h"
#include "DirectXBase.h"

class RenderTextureCommon {
public:
	// singletonインスタンスの取得
	static RenderTextureCommon* GetInstance();

	// 終了
	void Finalize();

	// 初期化
	void Initialize(DirectXBase* dxBase);

	// 描画前処理
	void PreDraw();

	// 描画後処理
	void PostDraw();

	// namespace省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	// レンダーテクスチャリソース
	ComPtr<ID3D12Resource> CreateRenderTextureResource(ComPtr<ID3D12Device> device, uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& color);

	// レンダーテクスチャの取得
	ComPtr<ID3D12Resource> GetRenderTextureResource() { return renderTextureResource_; }

	// コンストラクタに渡すための鍵
	class ConstructorKey {
	private:
		ConstructorKey() = default;
		friend class RenderTextureCommon;
	};

	// PassKeyを受け取るコンストラクタ
	explicit RenderTextureCommon(ConstructorKey) {}

private:
	// 深度バッファの生成
	void CreateDepthBuffer();

	// レンダーターゲットビューの初期化
	void RenderTargetViewInitialize();

	// 深度ステンシルビューの初期化
	void DepthStencilInitialize();

	// シングルトンインスタンス
	static std::unique_ptr<RenderTextureCommon> instance_;

	// DirectX12デバイス
	ComPtr<ID3D12Device> device_;

	// コマンドリスト
	ComPtr<ID3D12GraphicsCommandList> commandList_;

	// DepthStencilTextureをウィンドウのサイズで作成
	ComPtr<ID3D12Resource> depthStencilResource_;

	// WindowsAPI
	WinApp* winApp_ = nullptr;

	// DirectXBase
	DirectXBase* dxBase_ = nullptr;

	// RTV用のヒープ
	ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_;

	// DSV用のヒープ
	ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_;

	// rtvデスク
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_{};

	// dsvデスク
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc_{};

	// ビューポート
	D3D12_VIEWPORT viewport_{};

	// シザー矩形
	D3D12_RECT scissorRect_{};

	// RTVHandle
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_{};
	// DSVHandle
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle_{};

	// レンダーテクスチャリソース
	ComPtr<ID3D12Resource> renderTextureResource_;

	// クリア値
	D3D12_CLEAR_VALUE clearValue_{};

	// SRVのインデックス
	uint32_t srvIndex_ = 0;

	~RenderTextureCommon() = default;
	RenderTextureCommon(const RenderTextureCommon&) = delete;
	const RenderTextureCommon& operator=(const RenderTextureCommon&) = delete;

	// default_delete にアクセスを許可する
	friend struct std::default_delete<RenderTextureCommon>;
};
