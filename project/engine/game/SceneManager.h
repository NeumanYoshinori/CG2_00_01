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

	// コンストラクタに渡すための鍵
	class ConstructorKey {
	private:
		ConstructorKey() = default;
		friend class SceneManager;
	};

	// PassKeyを受け取るコンストラクタ
	explicit SceneManager(ConstructorKey) {}

private:
	// インスタンス
	static std::unique_ptr<SceneManager> instance_;

	// 今のシーン（実行中シーン）
	std::unique_ptr<BaseScene> scene_;

	// 次のシーン
	std::unique_ptr<BaseScene> nextScene_;

	// シーンファクトリー（借りてくる）
	AbstractSceneFactory* sceneFactory_ = nullptr;

	~SceneManager() = default;
	SceneManager(const SceneManager&) = delete;
	const SceneManager& operator=(const SceneManager&) = delete;

	// default_delete にアクセスを許可する
	friend struct std::default_delete<SceneManager>;
};

