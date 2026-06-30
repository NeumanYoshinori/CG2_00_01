#pragma once
#include "DirectXBase.h"

class PostEffect {
public:
	// シングルトンインスタンスの取得
	static PostEffect* GetInstance();

	// 解放
	void Finalize();

	// 初期化
	void Initialize(DirectXBase* dxBase);

	// 描画
	void Draw();

private:
	// ルートシグネチャの作成
	void CreateRootSignature();

	// グラフィックスパイプラインの生成
	void GenerateGraphicsPipeline();

	// シングルトンインスタンス
	static PostEffect* instance;

	// DxBase
	DirectXBase* dxBase_ = nullptr;

	// ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

	// グラフィックスパイプライン
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;

	// SRVインデックス
	uint32_t srvIndex_;

	// レンダーテクスチャリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> renderTextureResource_;

	PostEffect() = default;
	~PostEffect() = default;
	PostEffect(const PostEffect&) = delete;
	const PostEffect& operator=(const PostEffect&) = delete;
};


