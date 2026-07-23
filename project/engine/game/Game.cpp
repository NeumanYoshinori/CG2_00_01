#include "Game.h"
#include "SceneFactory.h"
#include "RenderTextureCommon.h"
#include "PostEffect.h"

void Game::Initialize() {
	// 基底クラスの初期化処理
	Framework::Initialize();
	
	// シーンファクトリを生成し、マネージャにセット
	sceneFactory_ = std::make_unique<SceneFactory>();
	SceneManager::GetInstance()->SetSceneFactory(sceneFactory_.get());
	// シーンマネージャに最初のシーンをセット
	SceneManager::GetInstance()->ChangeScene("TITLE");
}

void Game::Update() {
	// 基底クラスの更新処理
	Framework::Update();
}

void Game::Draw() {
	// 描画前処理
	RenderTextureCommon::GetInstance()->PreDraw();

	// シーン描画
	SceneManager::GetInstance()->Draw();

	// 描画後処理
	RenderTextureCommon::GetInstance()->PostDraw();

	// 描画前処理
	DirectXBase::GetInstance()->PreDraw();
	
	// ポストエフェクト描画
	PostEffect::GetInstance()->Draw();

	// ImGui描画
	SceneManager::GetInstance()->ImGuiDraw();

	// 描画後処理
	DirectXBase::GetInstance()->PostDraw();
}

void Game::Finalize() {
	// 基底クラスの終了処理
	Framework::Finalize();
}

