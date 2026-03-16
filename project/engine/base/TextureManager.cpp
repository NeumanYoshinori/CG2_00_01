#include "TextureManager.h"
#include "StringUtility.h"
#include <cassert>

using namespace DirectX;
using namespace std;
using namespace StringUtility;
using namespace Microsoft::WRL;

TextureManager* TextureManager::instance = nullptr;

// ImGuiで0番を使用するため、1番から使用
uint32_t TextureManager::kSRVIndexTop = 1;

TextureManager* TextureManager::GetInstance() {
	if (instance == nullptr) {
		instance = new TextureManager;
	}
	return instance;
}

void TextureManager::Finalize() {
	delete instance;
	instance = nullptr;
}

void TextureManager::Initialize(DirectXBase* dxBase, SrvManager* srvManager) {
	// SRVの数と同数
	textureDatas.reserve(SrvManager::kMaxSRVCount);

	// メンバ変数に記録
	dxBase_ = dxBase;

	srvManager_ = srvManager;
}

void TextureManager::LoadTexture(const string& filePath) {
	// 読み込み済みテクスチャを検索
	if (textureDatas.contains(filePath)) {
		return;
	}

	// テクスチャ枚数上限チェック
	assert(srvManager_->CheckMax());

	// テクスチャファイルを読んでプログラムで扱えるようにする
	ScratchImage image{};
	wstring filePathW = ConvertString(filePath);
	HRESULT hr = LoadFromWICFile(filePathW.c_str(), WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// ミップマップの作成
	ScratchImage mipImages{};
	hr = GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	// テクスチャデータを追加して書き込む
	TextureData& textureData = textureDatas[filePath];
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = dxBase_->CreateTextureResource(textureData.metadata);
	// SRV確保
	textureData.srvIndex = srvManager_->Allocate();
	textureData.srvHandleCPU = srvManager_->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(textureData.srvIndex);

	srvManager_->CreateSRVforTexture2D(textureData.srvIndex, textureData.resource.Get(), textureData.metadata.format, UINT(textureData.metadata.mipLevels));

	// テクスチャデータ転送
	ComPtr<ID3D12Resource> intermediateResource = dxBase_->UploadTextureData(textureData.resource, mipImages);

	// コマンドリスト
	ComPtr<ID3D12GraphicsCommandList> commandList = dxBase_->GetCommandList();
	// コマンドキュー
	ComPtr<ID3D12CommandQueue> commandQueue = dxBase_->GetCommandQueue();
	// コマンドアロケーター
	ComPtr<ID3D12CommandAllocator> commandAllocator = dxBase_->GetCommandAllocator();

	// コマンドリストの内容を確定させる。すべてのコマンドを積んでからCloseすること
	hr = commandList->Close();
	assert(SUCCEEDED(hr));

	ComPtr<ID3D12CommandList> commandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists->GetAddressOf());

	// フェンス
	ComPtr<ID3D12Fence> fence = dxBase_->GetFence();

	// フェンスイベント
	HANDLE fenceEvent = dxBase_->GetFenceEvent();

	// フェンス値
	uint64_t fenceVal = 0;

	// Fenceの値を更新
	fenceVal++;
	// GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSignalを送る
	commandQueue->Signal(fence.Get(), fenceVal);

	// Fenceの値が指定したSinal値にたどり着いているか確認する
	// GetCompleteValueの初期値はFence作成時に渡した初期値
	if (fence->GetCompletedValue() < fenceVal) {
		// 指定したSignalにたどり着いていないので、たどり着くまで待つようイベントを設定する
		fence->SetEventOnCompletion(fenceVal, fenceEvent);
		// イベント待つ
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	// 次のフレーム用のコマンドリストを準備
	hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList->Reset(commandAllocator.Get(), nullptr);
	assert(SUCCEEDED(hr));
}

uint32_t TextureManager::GetTextureIndexByFilePath(const string& filePath) {
	// 読み込み済みテクスチャを検索
	auto it = textureDatas.find(filePath);
	if (it != textureDatas.end()) {
		// 読み込み済みなら要素番号を返す
		uint32_t textureIndex = static_cast<uint32_t>(distance(textureDatas.begin(), it));
		return textureIndex;
	}

	// テクスチャ枚数上限チェック
	assert(srvManager_->CheckMax());

	assert(0);
	return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const string& filePath) {
	// 範囲外指定違反チェック
	assert(textureDatas.contains(filePath));

	TextureData& textureData = textureDatas[filePath];
	return textureData.srvHandleGPU;
}

const TexMetadata& TextureManager::GetMetaData(const string& filePath) {
	// 範囲外指定違反チェック
	assert(textureDatas.contains(filePath));

	TextureData& textureData = textureDatas[filePath];
	return textureData.metadata;
}

uint32_t TextureManager::GetSrvIndex(const std::string& filePath) {
	// 範囲外指定違反チェック
	assert(textureDatas.contains(filePath));

	TextureData& textureData = textureDatas[filePath];
	return textureData.srvIndex;
}
