#include "Game.h"
#include "CrashHandler.h"
#include "Logger.h"

void Game::Initialize() {
	// 規定クラスの初期化処理
	Framework::Initialize();

	// ゲームプレイシーンの生成
	scene_ = new TitleScene();
	// ゲームプレイシーンの初期化
	scene_->Initialize();
}

void Game::Update() {
	// 基底クラスの更新処理
	Framework::Update();
	// シーンの更新処理
	scene_->Update();
}

void Game::Draw() {
	// 描画前処理
	DirectXBase::GetInstance()->PreDraw();

	// シーン描画
	scene_->Draw();

	// 描画後処理
	DirectXBase::GetInstance()->PostDraw();
}

void Game::Finalize() {
	// シーンの終了処理
	scene_->Finalize();
	// シーンの解放
	delete scene_;

	// 基底クラスの終了処理
	Framework::Finalize();
}

