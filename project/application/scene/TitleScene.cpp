#include "TitleScene.h"
#include "SceneManager.h"

using namespace std;

void TitleScene::Initialize() {
	// インスタンス取得
	input_ = Input::GetInstance();

	textureManager_ = TextureManager::GetInstance();
	textureManager_->LoadTexture("rostock_laage_airport_4k.dds");
	textureManager_->LoadTexture("WebHunter.png");

	imGuiManager_ = ImGuiManager::GetInstance();

	camera_ = make_unique<Camera>();
	camera_->SetRotate({ 0.0f, 1.75f, 0.0f });
	camera_->SetTranslate({ 0.0f, 0.0f, 0.0f });

	skyboxCommon_ = SkyboxCommon::GetInstance();
	skyboxCommon_->SetDefaultCamera(camera_.get());

	skybox_ = make_unique<Skybox>();
	skybox_->Initialize("rostock_laage_airport_4k.dds");

	spriteCommon_ = SpriteCommon::GetInstance();
	sprite_ = make_unique<Sprite>();
	sprite_->Initialize("WebHunter.png");
}

void TitleScene::Finalize() {
}

void TitleScene::Update() {
	// ENTERキーを押したら
	if (input_->TriggerKey(DIK_R)) {
		// シーン切り替え
		SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
	}

	camera_->Update();

	skybox_->Update();

	sprite_->Update();

	imGuiManager_->Begin();
	imGuiManager_->End();
}

void TitleScene::Draw() {
	// SRVマネージャ描画前処理
	SrvManager::GetInstance()->PreDraw();

	skyboxCommon_->DrawSetting();

	skybox_->Draw();

	spriteCommon_->DrawSetting();
	sprite_->Draw();
}

void TitleScene::ImGuiDraw() {
	// ImGui受付開始
	ImGuiManager::GetInstance()->Draw();
}
