#pragma once
#include "Input.h"
#include "WinApp.h"
#include "D3DResourceLeakChecker.h"
#include "SrvManager.h"
#include "Audio.h"
#include "SceneManager.h"

// ゲーム全体
class Framework {
public:
	// 初期化
	virtual void Initialize();

	// 終了
	virtual void Finalize();

	// 終了
	virtual void Update();

	// 毎フレーム更新
	virtual void Draw() = 0;

	// 終了チェック
	virtual bool IsEndRequest() { return endRequest_; }

	// 仮想デストラクタ
	virtual ~Framework() = default;

	// 実行
	void Run();

private:
	D3DResourceLeakChecker leakCheck;
	Microsoft::WRL::ComPtr<IDXGIFactory7> dcgiFactory;

	// WindowsAPI
	WinApp* winApp_ = nullptr;

	// DirectX基盤部分
	DirectXBase* dxBase_ = nullptr;

	// SRVマネージャ
	SrvManager* srvManager_ = nullptr;

	bool endRequest_ = false;

	// シーンマネージャ
	SceneManager* sceneManager_ = nullptr;
};

