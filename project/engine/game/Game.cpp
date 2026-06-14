#include "Game.h"
#include "CrashHandler.h"
#include "Logger.h"

void Game::Initialize() {
	// 基底クラスの初期化処理
	Framework::Initialize();

	// シーンマネージャのインスタンス取得
	sceneManager_ = SceneManager::GetInstance();

	// 最初のシーンの生成
	BaseScene* scene = new TitleScene();
	// シーンマネージャに最初のシーンをセット
	sceneManager_->SetNextScene(scene);
}

void Game::Update() {
	// 基底クラスの更新処理
	Framework::Update();
}

void Game::Draw() {
	// 描画前処理
	DirectXBase::GetInstance()->PreDraw();

	// シーン描画
	sceneManager_->Draw();

	// 描画後処理
	DirectXBase::GetInstance()->PostDraw();
}

void Game::Finalize() {
	// 基底クラスの終了処理
	Framework::Finalize();
}

