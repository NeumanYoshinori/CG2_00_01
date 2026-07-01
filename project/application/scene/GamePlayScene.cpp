#include "GamePlayScene.h"
#include <random>
#include <numbers>
#include "PostEffect.h"

using namespace std;
using namespace numbers;

void GamePlayScene::Initialize() {
	// インスタンス取得
	winApp_ = WinApp::GetInstance();

	dxBase_ = DirectXBase::GetInstance();

	input_ = Input::GetInstance();

	srvManager_ = SrvManager::GetInstance();

	textureManager_ = TextureManager::GetInstance();

	// テクスチャを読み込む
	textureManager_->LoadTexture("resources/uvChecker.png");
	textureManager_->LoadTexture("resources/monsterBall.png");
	textureManager_->LoadTexture("resources/rostock_laage_airport_4k.dds");

	// カメラの初期化
	camera_ = new Camera();
	camera_->SetRotate({ 0.0f, 1.75f, 0.0f });
	camera_->SetTranslate({ 0.0f, 0.0f, 0.0f });

	// スカイボックス共通部の初期化
	skyboxCommon_ = new SkyboxCommon();
	skyboxCommon_->Initialize(dxBase_);

	// スカイボックスの初期化
	skybox_ = new Skybox();
	skybox_->Initialize(skyboxCommon_, "resources/rostock_laage_airport_4k.dds");
	skybox_->SetCamera(camera_);

	// モデルマネージャのインスタンス取得
	modelManager_ = ModelManager::GetInstance();

	// 3Dオブジェクト基盤部分のインスタンス取得
	object3dCommon_ = Object3dCommon::GetInstance();

	// .objファイルからモデルを読み込む
	modelManager_->LoadModel("terrain.obj");

	// 3dオブジェクトの初期化
	terrain_ = new Object3d();
	terrain_->Initialize(object3dCommon_);

	// 初期化済みの3Dオブジェクトにモデルを紐づける
	terrain_->SetModel("terrain.obj");
	terrain_->SetCamera(camera_);
	terrain_->SetSkybox(skybox_);

	// 球の初期化
	sphere_ = new Sphere();
	sphere_->Initialize(object3dCommon_, "resources/monsterBall.png");
	sphere_->SetCamera(camera_);
	sphere_->SetSkybox(skybox_);

	// 乱数生成器の初期化
	randomEngine_ = mt19937(seedGenerator_());

	// パーティクルマネージャ
	particleManager_ = ParticleManager::GetInstance();
	particleManager_->Initialize(dxBase_, srvManager_, camera_);

	// 円のパーティクルグループを作成
	particleManager_->CreateParticleGroup(ParticleManager::Cylinder, "gradationLine", "resources/gradationLine.png");

	uniform_real_distribution<float> distScale(0.4f, 1.5f);
	uniform_real_distribution<float> distRotate(-pi_v<float>, pi_v<float>);

	// パーティクルエミッターの初期化
	particleTransform.scale = { 1.0f, 1.0f, 1.0f}; // 横に潰す
	particleTransform.rotate = { 0.0f, 0.0f, 0.0f};
	particleTransform.translate = { 5.0f, 0.0f, 0.0f };
	Vector3 particleVelocity = { 0.0f, 0.0f, 0.0f }; // 動かない
	Vector4 particleColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	particleEmitter_ = new ParticleEmitter("gradationLine", particleTransform, particleVelocity, particleColor, 1, 1.0f);

	// ImGuiマネージャの初期化
	imGuiManager_ = new ImGuiManager();
	imGuiManager_->Initialize(winApp_, dxBase_);

	// オーディオの初期化
	audio_ = Audio::GetInstance();
	audio_->Initialize();
	// 音声読み込み
	soundData2 = audio_->SoundLoadFile("resources/The_maze_of_aqua.mp3");
	// 音声再生
	audio_->SoundPlayWave(soundData2, true);
}

void GamePlayScene::Finalize() {
	// 3dオブジェクトの解放
	delete terrain_;
	terrain_ = nullptr;

	// 球の解放
	delete sphere_;
	sphere_ = nullptr;

	// パーティクルエミッターの解放
	delete particleEmitter_;
	particleEmitter_ = nullptr;

	// パーティクルマネージャの終了
	particleManager_->Finalize();

	// スカイボックスの解放
	delete skybox_;
	skybox_ = nullptr;

	// スカイボックス共通部の解放
	delete skyboxCommon_;
	skyboxCommon_ = nullptr;

	// カメラの解放
	delete camera_;
	camera_ = nullptr;

	// ImGuiマネージャの終了処理
	imGuiManager_->Finalize();

	// ImGuiマネージャの解放
	delete imGuiManager_;
	imGuiManager_ = nullptr;

	// オーディオの終了
	audio_->Release();

	// 音声データ開放
	audio_->SoundUnload(&soundData2);
}

void GamePlayScene::Update() {
	// 0キーを押したときコンソールにHit 0と表示する
	if (input_->ReleaseKey(DIK_0)) {
		OutputDebugStringA("Hit 0\n");
	}

	// カメラの更新
	camera_->Update();

	// スカイボックスの更新
	skybox_->Update();

	// 3Dオブジェクトの更新
	terrain_->Update();

	// パーティクルマネージャの更新
	particleManager_->Update();

	// パーティクルエミッターの更新
	particleEmitter_->Update();

	// 球の更新
	sphere_->Update();

	// ImGui受付開始
	imGuiManager_->Begin();

#ifdef USE_IMGUI
	// デモウィンドウの表示オン
	ImGui::ShowDemoWindow();

	ImGui::Begin("Settings");
	camera_->DebugUpdate();
	Vector3 terrainPos = terrain_->GetTranslate();
	ImGui::DragFloat3("terrainPos", &terrainPos.x, 0.01f);
	terrain_->SetTranslate(terrainPos);
	Vector3 terrainRot = terrain_->GetRotate();
	ImGui::DragFloat3("terrainRot", &terrainRot.x, 0.01f);
	terrain_->SetRotate(terrainRot);
	Vector3 spherePos = sphere_->GetTranslate();
	ImGui::DragFloat3("spherePos", &spherePos.x, 0.01f);
	sphere_->SetTranslate(spherePos);
	Vector3 sphereRot = sphere_->GetRotate();
	ImGui::DragFloat3("sphereRot", &sphereRot.x, 0.01f);
	sphere_->SetRotate(sphereRot);
	Vector3 skyboxPos = skybox_->GetTranslate();
	ImGui::DragFloat3("skyboxPos", &skyboxPos.x, 0.01f);
	skybox_->SetTranslate(skyboxPos);
	LightManager::GetInstance()->DebugPointLight();
	float environmentCoefficient = sphere_->GetEnvironmentCoefficient();
	ImGui::DragFloat("environmentCoefficient", &environmentCoefficient, 0.01f);
	sphere_->SetEnvironmentCoefficient(environmentCoefficient);
	static PostEffect::PostEffectType currentPostEffect = PostEffect::PostEffectType::FullScreen;
	const char* postEffect[] = { "FullScreen", "Grayscale", "Vignette" };
	if (ImGui::BeginCombo("PostEffect", postEffect[static_cast<int>(currentPostEffect)])) {
		for (uint32_t i = 0; i < size(postEffect); ++i) {
			const bool isSelected = (static_cast<int>(currentPostEffect) == i);
			if (ImGui::Selectable(postEffect[i], isSelected)) {
				currentPostEffect = static_cast<PostEffect::PostEffectType>(i);

				PostEffect::GetInstance()->SetPostEffect(currentPostEffect);

				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
		}
		ImGui::EndCombo();
	}
	ImGui::Checkbox("useSepia", &useSepia);
	PostEffect::GetInstance()->UseSepia(useSepia);
	if (ImGui::InputFloat3("GrayscaleRGB", rgb)) {
		PostEffect::GetInstance()->SetColor(Vector3(rgb[0], rgb[1], rgb[2]));
	}
	ImGui::End();
#endif

	// ImGui受付終了
	imGuiManager_->End();
}

void GamePlayScene::Draw() {
	// SRVマネージャの描画前処理
	srvManager_->PreDraw();

	// スカイボックスの描画準備
	skyboxCommon_->DrawSetting();

	// スカイボックスの描画
	skybox_->Draw();

	// 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	object3dCommon_->DrawSetting();

	// 3Dオブジェクトの描画
	terrain_->Draw();

	// 球の描画
	sphere_->Draw();

	// パーティクルマネージャ描画
	particleManager_->Draw();
}

void GamePlayScene::ImGuiDraw() {
	// ImGuiの描画
	imGuiManager_->Draw();
}
