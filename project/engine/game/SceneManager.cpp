#include "SceneManager.h"
#include <cassert>

using namespace std;

unique_ptr<SceneManager> SceneManager::instance_ = nullptr;

SceneManager* SceneManager::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = make_unique<SceneManager>(ConstructorKey());
	}
	return instance_.get();
}

void SceneManager::Finalize() {
	// 最後のシーン終了と解放
	scene_->Finalize();

	instance_.reset();
}

void SceneManager::ChangeScene(const std::string& sceneName) {
	assert(sceneFactory_);
	assert(nextScene_ == nullptr);

	// 次シーンを生成
	nextScene_ = sceneFactory_->CreateScene(sceneName);
}

void SceneManager::Update() {
	// 次シーンの予約があるなら
	if (nextScene_) {
		// 旧シーンの終了
		if (scene_) {
			scene_->Finalize();
		}

		// シーン切り替え
		scene_ = move(nextScene_);
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

void SceneManager::ImGuiDraw() {
	// シーンの描画呼び出し
	scene_->ImGuiDraw();
}
