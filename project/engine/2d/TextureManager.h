#pragma once
#include <externals/DirectXTex/DirectXTex.h>
#include <wrl.h>
#include <d3d12.h>
#include <cstdint>
#include <string>
#include "DirectXBase.h"
#include "SrvManager.h"
#include <unordered_map>

// テクスチャマネージャー
class TextureManager {
public:
	// シングルトンインスタンスの取得
	static TextureManager* GetInstance();
	// 終了
	void Finalize();

	// 初期化
	void Initialize(DirectXBase* dxBase, SrvManager* srvManager);

	// テクスチャファイルの読み込み
	void LoadTexture(const std::string& filePath);

	// SRVインデックスの開始番号
	uint32_t GetTextureIndexByFilePath(const std::string& filePath);

	// メタデータを取得
	const DirectX::TexMetadata& GetMetaData(const std::string& filePath);
	// SRVインデックスの取得
	uint32_t GetSrvIndex(const std::string& filePath);
	// テクスチャ番号からCPUハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(const std::string& filePath);

private:
	// テクスチャ1枚分のデータ
	struct TextureData {
		DirectX::TexMetadata metadata;
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		uint32_t srvIndex;
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;
	};

	// インスタンス
	static TextureManager* instance;

	// テクスチャデータ
	std::unordered_map<std::string, TextureData> textureDatas;

	// DirectXBase
	DirectXBase* dxBase_ = nullptr;

	// SRVインデックスの開始番号
	static uint32_t kSRVIndexTop;

	// SRVマネージャ
	SrvManager* srvManager_ = nullptr;

	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = delete;
	TextureManager& operator=(TextureManager&) = delete;
};

