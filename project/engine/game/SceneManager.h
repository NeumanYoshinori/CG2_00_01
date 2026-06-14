#pragma once
#include "BaseScene.h"

// シーン管理
class SceneManager {
public:
	// シングルトンインスタンスの取得
	static SceneManager* GetInstance();

	// 終了
	void Finalize();

	// 次シーン予約
	void SetNextScene(BaseScene* nextScene) { nextScene_ = nextScene; }

	// 更新
	void Update();

	// 描画
	void Draw();

private:
	// インスタンス
	static SceneManager* instance;

	// 今のシーン（実行中シーン）
	BaseScene* scene_ = nullptr;

	// 次のシーン
	BaseScene* nextScene_ = nullptr;

	SceneManager() = default;
	~SceneManager() = default;
	SceneManager(const SceneManager&) = delete;
	const SceneManager& operator=(const SceneManager&) = delete;
};

