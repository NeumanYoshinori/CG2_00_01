#include "TextureManager.h"
#include "DirectXBase.h"
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

void TextureManager::Initialize(DirectXBase* dxBase) {
	// SRVの数と同数
	textureDatas.reserve(DirectXBase::kMaxSRVCount);

	// メンバ変数に記録
	dxBase_ = dxBase;
}

void TextureManager::LoadTexture(const string& filePath) {
	// 読み込み済みテクスチャを検索
	auto it = std::find_if(
		textureDatas.begin(),
		textureDatas.end(),
		[&](TextureData& textureData) { return textureData.filePath == filePath; }
	);
	if (it != textureDatas.end()) {
		// 読み込み済みなら早期return
		return;
	}

	// テクスチャ枚数上限チェック
	assert(textureDatas.size() + kSRVIndexTop < DirectXBase::kMaxSRVCount);

	// テクスチャファイルを読んでプログラムで扱えるようにする
	ScratchImage image{};
	wstring filePathW = ConvertString(filePath);
	HRESULT hr = LoadFromWICFile(filePathW.c_str(), WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// ミップマップの作成
	ScratchImage mipImages{};
	hr = GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	// テクスチャデータを追加
	textureDatas.resize(textureDatas.size() + 1);
	// 追加したテクスチャデータの参照を取得する
	TextureData& textureData = textureDatas.back();
	textureData.filePath = filePath;
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = dxBase_->CreateTextureResource(textureData.metadata);

	// テクスチャデータの要素数番号をSRVのインデックスとする
	uint32_t srvIndex = static_cast<uint32_t>(textureDatas.size() - 1) + kSRVIndexTop;

	textureData.srvHandleCPU = dxBase_->GetSRVCPUDescriptorHandle(srvIndex);
	textureData.srvHandleGPU = dxBase_->GetSRVGPUDescriptorHandle(srvIndex);

	// metadataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureData.metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(textureData.metadata.mipLevels);
	// 設定をもとにSRVの生成
	dxBase_->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);

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
	// 読み込み済みテクスチャデータを検索
	auto it = std::find_if(
		textureDatas.begin(),
		textureDatas.end(),
		[&](TextureData& textureData) { return textureData.filePath == filePath; }
	);
	if (it != textureDatas.end()) {
		// 読み込み済みなら要素番号を返す
		uint32_t textureIndex = static_cast<uint32_t>(distance(textureDatas.begin(), it));
		return textureIndex;
	}

	assert(0);
	return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(uint32_t textureIndex) {
	// 範囲外指定違反チェック
	assert(textureIndex < textureDatas.size());

	TextureData& textureData = textureDatas[textureIndex];
	return textureData.srvHandleGPU;
}
