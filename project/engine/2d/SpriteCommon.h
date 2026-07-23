#pragma once
#include <d3d12.h>
#include "DirectXBase.h"

// スプライト共通部
class SpriteCommon {
public: // メンバ関数
	// インスタンスの取得
	static SpriteCommon* GetInstance();

	// 終了
	void Finalize();

	// 初期化
	void Initialize();

	// 共通描画設定
	void DrawSetting();

	// コンストラクタに渡すための鍵
	class ConstructorKey {
	private:
		ConstructorKey() = default;
		friend class SpriteCommon;
	};

	// PassKeyを受け取るコンストラクタ
	explicit SpriteCommon(ConstructorKey) {}

private:
	// ルートシグネチャの作成
	void CreateRootSignature();
	// グラフィックスパイプラインの生成
	void GenerateGraphicsPipeLine();

	// インスタンス
	static std::unique_ptr<SpriteCommon> instance_;

	// DirectXBase
	DirectXBase* dxBase_ = nullptr;

	// コマンドリストを生成する
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;

	// ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

	// グラフィックスパイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;

	~SpriteCommon() = default;
	SpriteCommon(SpriteCommon&) = delete;
	SpriteCommon& operator=(SpriteCommon&) = delete;

	// default_delete にアクセスを許可する
	friend struct std::default_delete<SpriteCommon>;
};

