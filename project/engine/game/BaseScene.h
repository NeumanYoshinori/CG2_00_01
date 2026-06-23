#pragma once

// 前方宣言
class SceneManager;

// シーン基底クラス
class BaseScene {
public: // メンバ関数
	// 初期化
	virtual void Initialize() = 0;

	// 終了
	virtual void Finalize() = 0;

	// 更新
	virtual void Update() = 0;

	// 描画
	virtual void Draw() = 0;

	// setter
	virtual void SetSceneManager(SceneManager* sceneManager) { sceneManager_ = sceneManager; }

	// 仮想デストラクタ
	virtual ~BaseScene() = default;

private:
	// シーンマネージャ
	SceneManager* sceneManager_ = nullptr;
};

