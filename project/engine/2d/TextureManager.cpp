#include "TextureManager.h"
#include "StringUtility.h"
#include <cassert>

using namespace DirectX;
using namespace std;
using namespace StringUtility;
using namespace Microsoft::WRL;

unique_ptr<TextureManager> TextureManager::instance_ = nullptr;

TextureManager* TextureManager::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = make_unique<TextureManager>(ConstructorKey());
	}
	return instance_.get();
}

void TextureManager::Finalize() {
	instance_.reset();
}

void TextureManager::Initialize() {
	// SRVの数と同数
	textureDatas_.reserve(SrvManager::kMaxSRVCount_);

	// メンバ変数に記録
	dxBase_ = DirectXBase::GetInstance();

	srvManager_ = SrvManager::GetInstance();
}

void TextureManager::LoadTexture(const string& filePath) {
	// 読み込み済みテクスチャを検索
	if (textureDatas_.contains(filePath)) {
		return;
	}

	// テクスチャ枚数上限チェック
	assert(srvManager_->CheckMax());

	// テクスチャファイルを読んでプログラムで扱えるようにする
	ScratchImage image{};
	wstring filePathW = ConvertString(filePath);

	HRESULT hr;
	if (filePathW.ends_with(L".dds")) { // .ddsで終わっていたらddsをみなす。
		hr = LoadFromDDSFile(filePathW.c_str(), DDS_FLAGS_NONE, nullptr, image);
	} else {
		hr = LoadFromWICFile(filePathW.c_str(), WIC_FLAGS_FORCE_SRGB, nullptr, image);
	}
	assert(SUCCEEDED(hr));

	// ミップマップの作成
	ScratchImage mipImages{};
	if (IsCompressed(image.GetMetadata().format)) { // 圧縮フォーマットかどうかを調べる
		mipImages = move(image); // 圧縮フォーマットならそのまま使うのでmoveする
	} else {
		hr = GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), TEX_FILTER_SRGB, 4, mipImages);
	}
	assert(SUCCEEDED(hr));

	// テクスチャデータを追加して書き込む
	TextureData& textureData = textureDatas_[filePath];
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = dxBase_->CreateTextureResource(textureData.metadata);
	// SRV確保
	textureData.srvIndex = srvManager_->Allocate();
	textureData.srvHandleCPU = srvManager_->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(textureData.srvIndex);

	srvManager_->CreateSRVforTexture2D(textureData.srvIndex, textureData.resource.Get(), textureData.metadata.format, UINT(textureData.metadata.mipLevels), textureData.metadata.IsCubemap());

	// テクスチャデータ転送
	ComPtr<ID3D12Resource> intermediateResource = dxBase_->UploadTextureData(textureData.resource, mipImages);

	// コマンドリスト
	ComPtr<ID3D12GraphicsCommandList> commandList = dxBase_->GetCommandList();
	// コマンドアロケーター
	ComPtr<ID3D12CommandAllocator> commandAllocator = dxBase_->GetCommandAllocator();

	// コマンドリストの内容を確定させる。すべてのコマンドを積んでからCloseすること
	hr = commandList->Close();
	assert(SUCCEEDED(hr));

	ID3D12CommandList* commandLists[] = { commandList.Get() };
	dxBase_->GetCommandQueue()->ExecuteCommandLists(_countof(commandLists), commandLists);

	// GPU完了待ち
	dxBase_->WaitForGPU();

	// 次のフレーム用のコマンドリストを準備
	hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList->Reset(commandAllocator.Get(), nullptr);
	assert(SUCCEEDED(hr));
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const string& filePath) {
	// 範囲外指定違反チェック
	assert(textureDatas_.contains(filePath));

	TextureData& textureData = textureDatas_[filePath];
	return textureData.srvHandleGPU;
}

const TexMetadata& TextureManager::GetMetaData(const string& filePath) {
	// 範囲外指定違反チェック
	assert(textureDatas_.contains(filePath));

	TextureData& textureData = textureDatas_[filePath];
	return textureData.metadata;
}

uint32_t TextureManager::GetSrvIndex(const std::string& filePath) {
	// 範囲外指定違反チェック
	assert(textureDatas_.contains(filePath));

	TextureData& textureData = textureDatas_[filePath];
	return textureData.srvIndex;
}
