#include "GamePlayScene.h"
#include <random>
#include <numbers>
#include "PostEffect.h"
#include "Vector4.h"
#include "SceneManager.h"
#include "Sphere.h"
#include "Plane.h"
#include "Ring.h"
#include "Cylinder.h"

using namespace std;
using namespace numbers;

void GamePlayScene::Initialize() {
	// インスタンス取得
	input_ = Input::GetInstance();

	textureManager_ = TextureManager::GetInstance();

	// テクスチャを読み込む
	textureManager_->LoadTexture("resources/uvChecker.png");
	textureManager_->LoadTexture("resources/monsterBall.png");
	textureManager_->LoadTexture("resources/rostock_laage_airport_4k.dds");
	textureManager_->LoadTexture("resources/gradationLine.png");

	// カメラの初期化
	camera_ = make_unique<Camera>();
	camera_->SetRotate({ 0.0f, 1.75f, 0.0f });
	camera_->SetTranslate({ 0.0f, 0.0f, 0.0f });

	// スカイボックス共通部の初期化
	skyboxCommon_ = SkyboxCommon::GetInstance();
	skyboxCommon_->SetDefaultCamera(camera_.get());

	// スカイボックスの初期化
	skybox_ = make_unique<Skybox>();
	skybox_->Initialize("resources/rostock_laage_airport_4k.dds");

	// モデルマネージャのインスタンス取得
	modelManager_ = ModelManager::GetInstance();

	// 3Dオブジェクト基盤部分のインスタンス取得
	object3dCommon_ = Object3dCommon::GetInstance();
	object3dCommon_->SetDefaultCamera(camera_.get());

	// .objファイルからモデルを読み込む
	modelManager_->LoadModel("terrain.obj");
	modelManager_->LoadModel("fence.obj");

	// 3dオブジェクトの初期化
	terrain_ = make_unique<Object3d>();
	terrain_->Initialize();

	// 初期化済みの3Dオブジェクトにモデルを紐づける
	terrain_->SetModel("terrain.obj");
	terrain_->SetSkybox(skybox_.get());

	// 球の初期化
	sphere_ = make_unique<Sphere>();
	sphere_->Initialize("resources/monsterBall.png", 1);

	// 平面の初期化
	plane_ = make_unique<Plane>();
	plane_->Initialize("resources/uvChecker.png", 1);

	ring_ = make_unique<Ring>();
	ring_->Initialize("resources/gradationLine.png", 1);

	cylinder_ = make_unique<Cylinder>();
	cylinder_->Initialize("resources/gradationLine.png", 1);

	// モンスターボールの初期化
	primitive_ = make_unique<Object3d>();
	primitive_->Initialize();

	// 初期化済みの3Dオブジェクトにプリミティブを紐づける
	primitive_->SetPrimitive(sphere_.get());
	primitive_->SetSkybox(skybox_.get());

	// 乱数生成器の初期化
	randomEngine_ = mt19937(seedGenerator_());

	particleManager_ = ParticleManager::GetInstance();
	particleManager_->SetCamera(camera_.get());

	// 円のパーティクルグループを作成
	particleManager_->CreateParticleGroup("Model", "fence", "fence.obj", false, false);
	particleManager_->CreateParticleGroup("Sphere", "cylinder", "resources/monsterBall.png", false, false);
	particleManager_->CreateParticleGroup("Plane", "circle", "resources/gradationLine.png", false, false);

	// パーティクルエミッターの初期化
	planeTransform.scale = {0.5f, 0.5f, 0.5f}; // 横に潰す
	planeTransform.rotate = { 0.0f, 0.0f, 0.0f};
	planeTransform.translate = { 5.0f, 0.0f, 0.0f };
	Vector3 particleVelocity = { 5.0f, 0.0f, 0.0f }; // 動かない
	Vector4 particleColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	fenceEmitter_ = make_unique<ParticleEmitter>("fence", planeTransform, particleVelocity, particleColor, 2.0f, 5, 2.0f);
	planeEmitter_ = make_unique<ParticleEmitter>("cylinder", planeTransform, particleVelocity, particleColor, 1.0f, 5, 2.0f);
	cylinderEmitter_ = make_unique<ParticleEmitter>("circle", planeTransform, particleVelocity, particleColor, 2.0f, 5, 2.0f);

	// ImGuiマネージャの初期化
	imGuiManager_ = ImGuiManager::GetInstance();

	// オーディオの初期化
	audio_ = Audio::GetInstance();
	// 音声読み込み
	soundData2 = audio_->SoundLoadFile("resources/audios/The_maze_of_aqua.mp3");
	// 音声再生
	bgmVoice_ = audio_->SoundPlayWave(soundData2, true);
}

void GamePlayScene::Finalize() {
	// 音声停止
	audio_->SoundStopWave(bgmVoice_);

	// 音声データ開放
	audio_->SoundUnload(&soundData2);
}

void GamePlayScene::Update() {
	// ENTERキーを押したら
	if (input_->TriggerKey(DIK_R)) {
		// シーン切り替え
		SceneManager::GetInstance()->ChangeScene("TITLE");
	}

	if (input_->TriggerKey(DIK_1)) {
		primitive_->SetPrimitive(sphere_.get());
	}
	if (input_->TriggerKey(DIK_2)) {
		primitive_->SetPrimitive(plane_.get());
	}
	if (input_->TriggerKey(DIK_3)) {
		primitive_->SetPrimitive(ring_.get());
	}
	if (input_->TriggerKey(DIK_4)) {
		primitive_->SetPrimitive(cylinder_.get());
	}

	// カメラの更新
	camera_->Update();

	// スカイボックスの更新
	skybox_->Update();

	// 地面の更新
	terrain_->Update();

	// モンスターボールの更新
	primitive_->Update();

	// パーティクルマネージャの更新
	particleManager_->Update();

	// パーティクルエミッターの更新
	fenceEmitter_->Update();
	planeEmitter_->Update();
	cylinderEmitter_->Update();

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

	// 地面のImGui
	if (ImGui::CollapsingHeader("Terrain")) {
		terrain_->DebugUpdate();
	}

	// 球のImGui
	if (ImGui::CollapsingHeader("Primitive")) {
		primitive_->DebugUpdate();
	}

	// スカイボックスのImGui
	if (ImGui::CollapsingHeader("Skybox")) {
		Vector3 skyboxPos = skybox_->GetTranslate();
		ImGui::DragFloat3("skyboxPos", &skyboxPos.x, 0.01f);
		skybox_->SetTranslate(skyboxPos);
	}

	if (ImGui::CollapsingHeader("Light")) {
		LightManager::GetInstance()->DebugLight();
	}

	if (ImGui::CollapsingHeader("PostEffect")) {
		PostEffect::GetInstance()->DebugUpdate();
	}

	ImGui::End();
#endif

	// ImGui受付終了
	imGuiManager_->End();
}

void GamePlayScene::Draw() {
	// SRVマネージャの描画前処理
	SrvManager::GetInstance()->PreDraw();

	// スカイボックスの描画準備
	skyboxCommon_->DrawSetting();

	// スカイボックスの描画
	skybox_->Draw();

	// 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	object3dCommon_->DrawSetting();

	// 地面の描画
	terrain_->Draw();

	// 球の描画
	primitive_->Draw();

	// パーティクルマネージャ描画
	particleManager_->Draw();
}

void GamePlayScene::ImGuiDraw() {
	// ImGuiの描画
	imGuiManager_->Draw();
}
