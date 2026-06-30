#pragma once
#include "BaseScene.h"
#include "AbstractSceneFactory.h"

// シーン管理
class SceneManager {
public:
	// シングルトンインスタンスの取得
	static SceneManager* GetInstance();

	// 終了
	void Finalize();

	/// <summary>
	/// 次シーン予約
	/// </summary>
	/// <param name="sceneName">シーン名</param>
	void ChangeScene(const std::string& sceneName);

	// 更新
	void Update();

	// 描画
	void Draw();

	// ImGui描画
	void ImGuiDraw();

	// シーンファクトリーのsetter
	void SetSceneFactory(AbstractSceneFactory* sceneFactory) { sceneFactory_ = sceneFactory; }

private:
	// インスタンス
	static SceneManager* instance;

	// 今のシーン（実行中シーン）
	BaseScene* scene_ = nullptr;

	// 次のシーン
	BaseScene* nextScene_ = nullptr;

	// シーンファクトリー（借りてくる）
	AbstractSceneFactory* sceneFactory_ = nullptr;

	SceneManager() = default;
	~SceneManager() = default;
	SceneManager(const SceneManager&) = delete;
	const SceneManager& operator=(const SceneManager&) = delete;
};

