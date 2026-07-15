#include "RenderTextureCommon.h"
#include <cassert>

using namespace std;
using namespace Microsoft::WRL;

unique_ptr<RenderTextureCommon> RenderTextureCommon::instance_ = nullptr;

RenderTextureCommon* RenderTextureCommon::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = make_unique<RenderTextureCommon>(ConstructorKey());
	}
	return instance_.get();
}

void RenderTextureCommon::Finalize() {
	instance_.reset();
}

void RenderTextureCommon::Initialize() {
	dxBase_ = DirectXBase::GetInstance();

	device_ = dxBase_->GetDevice();

	// 深度バッファの作成
	CreateDepthBuffer();

	// 各種デスクリプターヒープの作成
	rtvDescriptorHeap_ = dxBase_->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1, false);
	dsvDescriptorHeap_ = dxBase_->GetDsvDescriptorHeap();

	// レンダーターゲットビューの初期化
	RenderTargetViewInitialize();
	// 深度ステンシルの初期化
	DepthStencilInitialize();

	viewport_ = dxBase_->GetViewport();
	scissorRect_ = dxBase_->GetScissorRect();
}

void RenderTextureCommon::PreDraw() {
	D3D12_RESOURCE_BARRIER barrier{};
	// 今回のバリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// バリアを張る対象のリソース(現在のバックバッファに対して行う)
	barrier.Transition.pResource = renderTextureResource_.Get();
	// 遷移前(現在)のResourceState
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	// 遷移後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// コマンドリストを取得
	commandList_ = dxBase_->GetCommandList();

	// TransitionBarrierを張る
	commandList_->ResourceBarrier(1, &barrier);

	// 描画先のRTVを設定する
	commandList_->OMSetRenderTargets(1, &rtvHandle_, false, &dsvHandle_);

	// 指定した色で画面全体をクリアする
	commandList_->ClearRenderTargetView(rtvHandle_, clearValue_.Color, 0, nullptr);
	// 画面全体の深度をクリア
	commandList_->ClearDepthStencilView(dsvHandle_, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	commandList_->RSSetViewports(1, &viewport_); // viewportを設定
	commandList_->RSSetScissorRects(1, &scissorRect_); // Scissorを設定
}

void RenderTextureCommon::PostDraw() {
	D3D12_RESOURCE_BARRIER barrier{};
	// 今回のバリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Noneにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// バリアを張る対象のリソース(現在のバックバッファに対して行う)
	barrier.Transition.pResource = renderTextureResource_.Get();
	// 遷移前(現在)のResourceState
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	// 遷移後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// コマンドリストを取得
	commandList_ = dxBase_->GetCommandList();
	// TransitionBarrierを張る
	commandList_->ResourceBarrier(1, &barrier);
}

ComPtr<ID3D12Resource> RenderTextureCommon::CreateRenderTextureResource(uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& clearColor) {
	// 生成するResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width; // Textureの幅
	resourceDesc.Height = height; // Textureの高さ
	resourceDesc.MipLevels = 1; // mipmapの数
	resourceDesc.DepthOrArraySize = 1; // 奥行き or 配列Textureの配列数
	resourceDesc.Format = format; // Renderとして利用可能なフォーマット
	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント。1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2次元
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; // RenderTargetとして使う通知

	// 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM上に作る

	// クリア値
	clearValue_.Format = format;
	clearValue_.Color[0] = clearColor.x;
	clearValue_.Color[1] = clearColor.y;
	clearValue_.Color[2] = clearColor.z;
	clearValue_.Color[3] = clearColor.w;

	// Resourceの生成
	ComPtr<ID3D12Resource> resource;
	HRESULT hr = device_->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&clearValue_,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

void RenderTextureCommon::CreateDepthBuffer() {
	HRESULT hr;
	// 生成するResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = WinApp::kClientWidth; // Textureの幅
	resourceDesc.Height = WinApp::kClientHeight; // Heightの高さ
	resourceDesc.MipLevels = 1; // mipmapの数
	resourceDesc.DepthOrArraySize = 1; // 奥行き or 配列Textureの配列数
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // DepthStencilとして利用可能なフォーマット
	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント。1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DepthStencilとして使う通知

	// 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM上に作る

	// 深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f; // 1.0fでクリア
	depthClearValue.DepthStencil.Stencil = 0;
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // フォーマット

	hr = device_->CreateCommittedResource(
		&heapProperties, // Heapの設定
		D3D12_HEAP_FLAG_NONE, // Heapの特殊な設定
		&resourceDesc, // Heapの特殊な設定
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&depthStencilResource_));
	assert(SUCCEEDED(hr));
}

void RenderTextureCommon::RenderTargetViewInitialize() {
	rtvHandle_ = rtvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	// RTVの設定
	rtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // 出力結果をSRGBに変換して書き込む
	rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D; // 2dテクスチャとして書き込む
	const Vector4 kRenderTargetClearValue{ 1.0f, 0.0f, 0.0f, 1.0f };
	renderTextureResource_ = CreateRenderTextureResource(WinApp::kClientWidth, WinApp::kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, kRenderTargetClearValue);
	device_->CreateRenderTargetView(renderTextureResource_.Get(), &rtvDesc_, rtvHandle_);
}

void RenderTextureCommon::DepthStencilInitialize() {
	dsvHandle_ = dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	// DSVの設定
	dsvDesc_.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // Format
	dsvDesc_.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // 2dTexture
	// DSVHeapの先頭にDSVをつくる
	device_->CreateDepthStencilView(depthStencilResource_.Get(), &dsvDesc_, dsvHandle_);
}