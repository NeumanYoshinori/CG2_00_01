#pragma once
#include <wrl.h>
#include <d3d12.h>
#include "DirectXBase.h"
#include "Camera.h"

// スカイボックス共通部
class SkyboxCommon {
public: // メンバ関数
	// 初期化
	void Initialize(DirectXBase* dxBase);

	// 共通描画設定
	void DrawSetting();

	// DxBaseのgetter
	DirectXBase* GetDxBase() const { return dxBase_; }

	// setter
	void SetDefaultCamera(Camera* camera) { defaultCamera_ = camera; }
	// getter
	Camera* GetDefaultCamera() const { return defaultCamera_; }

private:
	// ルートシグネチャの作成
	void CreateRootSignature();
	// グラフィックスパイプラインの生成
	void GenerateGraphicsPipeLine();

	// DirectXBase
	DirectXBase* dxBase_;

	// コマンドリストを生成する
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;

	// ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;

	// グラフィックスパイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;

	// ブレンドデスク
	D3D12_BLEND_DESC blendDesc = {};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc = {};

	// デフォルトカメラ
	Camera* defaultCamera_ = nullptr;
};
