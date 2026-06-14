#pragma once
#include "Framework.h"

// 前方宣言
class SceneFactory;

// ゲーム全体
class Game : public Framework {
public: // メンバ関数
	// 初期化
	void Initialize() override;

	// 終了
	void Finalize() override;

	// 毎フレーム更新
	void Update() override;

	// 描画
	void Draw() override;

private:
	// シーンファクトリー
	SceneFactory* sceneFactory_ = nullptr;
};

