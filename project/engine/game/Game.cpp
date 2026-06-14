#include "Game.h"
#include "CrashHandler.h"
#include "Logger.h"
#include "SceneFactory.h"

void Game::Initialize() {
	// 基底クラスの初期化処理
	Framework::Initialize();

	// シーンファクトリを生成し、マネージャにセット
	sceneFactory_ = new SceneFactory();
	SceneManager::GetInstance()->SetSceneFactory(sceneFactory_);
	// シーンマネージャに最初のシーンをセット
	SceneManager::GetInstance()->ChangeScene("TITLE");
}

void Game::Update() {
	// 基底クラスの更新処理
	Framework::Update();
}

void Game::Draw() {
	// 描画前処理
	DirectXBase::GetInstance()->PreDraw();

	// シーン描画
	SceneManager::GetInstance()->Draw();

	// 描画後処理
	DirectXBase::GetInstance()->PostDraw();
}

void Game::Finalize() {
	// 基底クラスの終了処理
	Framework::Finalize();
}

