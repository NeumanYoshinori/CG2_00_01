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
	void Initialize(DirectXBase* dxBase);

	// 共通描画設定
	void DrawSetting();

	// DxBaseのgetter
	DirectXBase* GetDxBase() const { return dxBase_; }

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
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;

	// ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;

	// グラフィックスパイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_ = nullptr;

	~SpriteCommon() = default;
	SpriteCommon(SpriteCommon&) = delete;
	SpriteCommon& operator=(SpriteCommon&) = delete;

	// default_delete にアクセスを許可する
	friend struct std::default_delete<SpriteCommon>;
};

