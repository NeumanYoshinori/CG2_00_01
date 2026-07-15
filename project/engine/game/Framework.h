#pragma once
#include "Input.h"
#include "D3DResourceLeakChecker.h"
#include "SceneManager.h"
#include "AbstractSceneFactory.h"
#include <dxgi1_6.h>

#pragma comment(lib, "dxgi.lib")

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

	bool endRequest_ = false;

	// シーンマネージャ
	SceneManager* sceneManager_ = nullptr;

protected:
	// シーンファクトリー
	std::unique_ptr<AbstractSceneFactory> sceneFactory_;
};

