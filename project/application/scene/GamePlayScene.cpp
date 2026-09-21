#include "GamePlayScene.h"
#include "PostEffect.h"
#include "SceneManager.h"
#include "Sphere.h"
#include "MathFunction.h"

using namespace std;
using namespace MathFunction;

void GamePlayScene::Initialize() {
	// インスタンス取得
	input_ = Input::GetInstance();

	textureManager_ = TextureManager::GetInstance();

	// テクスチャを読み込む
	textureManager_->LoadTexture("resources/monsterBall.png");
	textureManager_->LoadTexture("resources/rostock_laage_airport_4k.dds");
	textureManager_->LoadTexture("resources/crosshair138.png");

	// カメラの初期化
	camera_ = make_unique<Camera>();

	// スカイボックス共通部の初期化
	skyboxCommon_ = SkyboxCommon::GetInstance();
	skyboxCommon_->SetDefaultCamera(camera_.get());

	// スカイボックスの初期化
	skybox_ = make_unique<Skybox>();
	skybox_->Initialize("resources/rostock_laage_airport_4k.dds");

	// モデルマネージャのインスタンス取得
	modelManager_ = ModelManager::GetInstance();
	modelManager_->LoadModel("axis.obj");

	// 3Dオブジェクト基盤部分のインスタンス取得
	object3dCommon_ = Object3dCommon::GetInstance();
	object3dCommon_->SetDefaultCamera(camera_.get());

	// 球の初期化
	sphere_ = make_unique<Sphere>();
	sphere_->Initialize("resources/monsterBall.png", 1);

	spriteCommon_ = SpriteCommon::GetInstance();

	// 銃の初期化
	gun_ = make_unique<Gun>();
	gun_->Initialize(camera_.get(), "resources/rostock_laage_airport_4k.dds", sphere_.get(), camera_->GetTranslate());

	// レティクルの初期化
	crosshair_ = make_unique<Sprite>();
	crosshair_->Initialize("resources/crosshair138.png");
	crosshair_->SetAnchorPoint({ 0.5f, 0.5f });

	// ImGuiマネージャの初期化
	imGuiManager_ = ImGuiManager::GetInstance();

	// オーディオの初期化
	audio_ = Audio::GetInstance();
	audio_->SoundLoadFile("resources/audios/gunShoot.mp3");
	audio_->SoundLoadFile("resources/audios/The_maze_of_aqua.mp3");
}

void GamePlayScene::Finalize() {
	audio_->SoundUnload("resources/audios/gunShoot.mp3");

	audio_->SoundStopWave(bgm_);
	audio_->SoundUnload("resources/audios/The_maze_of_aqua.mp3");
}

void GamePlayScene::Update() {
	// ENTERキーを押したら
	if (input_->TriggerKey(DIK_R)) {
		// シーン切り替え
		SceneManager::GetInstance()->ChangeScene("TITLE");
	}

	if (!bgm_) {
		bgm_ = audio_->SoundPlayWave("resources/audios/The_maze_of_aqua.mp3", true, 1.0f);
	}

	// カメラの更新
	camera_->Update();

	// スカイボックスの更新
	skybox_->Update();

	// マウス座標を取得
	GetCursorPos(&mousePosition_);

	// クライエントに変換
	ScreenToClient(WinApp::GetInstance()->GetHwnd(), &mousePosition_);

	// 銃の更新
	gun_->Update(mousePosition_);

	// スプライトの座標をマウス位置に設定
	crosshair_->SetPosition({ static_cast<float>(mousePosition_.x), static_cast<float>(mousePosition_.y) });
	// スプライトの更新
	crosshair_->Update();

	// ImGui受付開始
	imGuiManager_->Begin();

#ifdef USE_IMGUI
	// デモウィンドウの表示オン
	ImGui::ShowDemoWindow();

	ImGui::Begin("Setting");

	// カメラのImGui
	if (ImGui::CollapsingHeader("Camera")) {
		camera_->DebugUpdate();
	}

	// 銃のデバッグ
	gun_->DebugUpdate();

	if (ImGui::CollapsingHeader("Light")) {
		LightManager::GetInstance()->DebugLight();
	}

	if (ImGui::CollapsingHeader("PostEffect")) {
		PostEffect::GetInstance()->DebugUpdate();
	}

	if (ImGui::CollapsingHeader("Crosshair")) {
		crosshair_->DebugUpdate();
	}

	ImGui::End();
#endif

	// ImGui受付終了
	imGuiManager_->End();
}

void GamePlayScene::Draw() {
	// SRVマネージャの描画前処理
	SrvManager::GetInstance()->PreDraw();

	// スカイボックス描画前処理
	skyboxCommon_->DrawSetting();

	// スカイボックスの描画
	skybox_->Draw();

	// 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	object3dCommon_->DrawSetting();

	// 銃の描画
	gun_->Draw();

	// スプライト描画前処理
	spriteCommon_->GetInstance()->DrawSetting();

	// スプライトの描画
	crosshair_->Draw();
}

void GamePlayScene::ImGuiDraw() {
	// ImGuiの描画
	imGuiManager_->Draw();
}
