#include "SceneManager.h"

SceneManager* SceneManager::instance = nullptr;

SceneManager* SceneManager::GetInstance() {
	if (instance == nullptr) {
		instance = new SceneManager;
	}
	return instance;
}

void SceneManager::Finalize() {
	// 最後のシーン終了と解放
	scene_->Finalize();
	delete scene_;

	delete instance;
	instance = nullptr;
}

void SceneManager::Update() {
	// 次シーンの予約があるなら
	if (nextScene_) {
		// 旧シーンの終了
		if (scene_) {
			scene_->Finalize();
			delete scene_;
		}

		// シーン切り替え
		scene_ = nextScene_;
		nextScene_ = nullptr;

		// シーンマネージャをセット
		scene_->SetSceneManager(this);

		// 次シーンを初期化する
		scene_->Initialize();
	}

	// 実行中シーンを更新する
	scene_->Update();
}

void SceneManager::Draw() {
	// シーンの描画呼び出し
	scene_->Draw();
}
