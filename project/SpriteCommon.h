#pragma once
#include <wrl.h>
#include <d3d12.h>
#include "DirectXBase.h"

// スプライト共通部
class SpriteCommon {
public:
	// 初期化
	void Initialize(DirectXBase* dxBase);

	// 共通描画設定
	void DrawSetting();

	DirectXBase* GetDxBase() const { return dxBase_; }

private:
	// ルートシグネチャの作成
	void CreateRootSignature();
	// グラフィックスパイプラインの生成
	void GenerateGraphicsPipeLine();

	DirectXBase* dxBase_;

	// コマンドリストを生成する
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = dxBase_->GetCommandList();

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
};

