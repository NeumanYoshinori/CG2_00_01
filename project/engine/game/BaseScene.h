#pragma once

// 前方宣言
class SceneManager;

// シーン基底クラス
class BaseScene {
public: // メンバ関数
	// 初期化
	virtual void Initialize();

	// 終了
	virtual void Finalize();

	// 更新
	virtual void Update();

	// 描画
	virtual void Draw();

	// setter
	virtual void SetSceneManager(SceneManager* sceneManager) { sceneManager = sceneManager_; }

	// 仮想デストラクタ
	virtual ~BaseScene() = default;

private:
	SceneManager* sceneManager_ = nullptr;
};

