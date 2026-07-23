#pragma once
#include <wrl.h>
#include <d3d12.h>
#include "DirectXBase.h"
#include "Camera.h"

// スカイボックス共通部
class SkyboxCommon {
public: // メンバ関数
	// シングルトンインスタンスの取得
	static SkyboxCommon* GetInstance();

	// 解放.
	void Finalize();

	// 初期化
	void Initialize();

	// 共通描画設定
	void DrawSetting();

	// setter
	void SetDefaultCamera(Camera* camera) { defaultCamera_ = camera; }
	// getter
	Camera* GetDefaultCamera() const { return defaultCamera_; }

	// コンストラクタに渡すための鍵
	class ConstructorKey {
	private:
		ConstructorKey() = default;
		friend class SkyboxCommon;
	};

	// PassKeyを受け取るコンストラクタ
	explicit SkyboxCommon(ConstructorKey) {}

private:
	// ルートシグネチャの作成
	void CreateRootSignature();
	// グラフィックスパイプラインの生成
	void GenerateGraphicsPipeLine();

	// シングルトンインスタンス
	static std::unique_ptr<SkyboxCommon> instance_;

	// DirectXBase
	DirectXBase* dxBase_ = nullptr;

	// コマンドリストを生成する
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;

	// ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;

	// グラフィックスパイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_ = nullptr;

	// ブレンドデスク
	D3D12_BLEND_DESC blendDesc_ = {};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc_ = {};

	// デフォルトカメラ
	Camera* defaultCamera_ = nullptr;

	~SkyboxCommon() = default;
	SkyboxCommon(SkyboxCommon&) = delete;
	SkyboxCommon& operator=(SkyboxCommon&) = delete;

	// default_delete にアクセスを許可する
	friend struct std::default_delete<SkyboxCommon>;
};
