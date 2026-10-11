#pragma once
#include "DirectXBase.h"

// 線共通部
class LineCommon {
public: // メンバ関数
	// インスタンスの取得
	static LineCommon* GetInstance();

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
		friend class LineCommon;
	};

	// PassKeyを受け取るコンストラクタ
	explicit LineCommon(ConstructorKey) {}

private:
	// ルートシグネチャ作成
	void CreateRootSignature();

	// グラフィックスパイプライン生成
	void GenerateGraphicsPipeline();

	static std::unique_ptr<LineCommon> instance_;

	DirectXBase* dxBase_ = nullptr;

	// コマンドリストを生成する
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;

	// ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

	// グラフィックスパイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;

	~LineCommon() = default;
	LineCommon(LineCommon&) = delete;
	LineCommon& operator=(LineCommon&) = delete;

	// default_delete にアクセスを許可する
	friend struct std::default_delete<LineCommon>;
};

