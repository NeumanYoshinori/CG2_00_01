#include "Game.h"
#include "CrashHandler.h"
#include "Logger.h"

void Game::Initialize() {
	// 誰も補足しなかった場合に(Unhandled)、補足する関数を登録
	SetUnhandledExceptionFilter(CrashHandler::ExportDump);

	// ログの生成
	Logger::GenerateLog();

	// WindowsAPIの初期化
	winApp_ = new WinApp();
	winApp_->Initialize();

	// DirectXの初期化
	dxBase_ = new DirectXBase();
	dxBase_->Initialize(winApp_);

	// 入力の初期化
	input_ = new Input();
	input_->Initialize(winApp_);

	// SRVマネージャの初期化
	srvManager_ = SrvManager::GetInstance();
	srvManager_->Initialize(dxBase_);

	// テクスチャマネージャの初期化
	textureManager_ = TextureManager::GetInstance();
	textureManager_->Initialize(dxBase_, srvManager_);

	// テクスチャを読み込む
	textureManager_->LoadTexture("resources/uvChecker.png");
	textureManager_->LoadTexture("resources/monsterBall.png");
	textureManager_->LoadTexture("resources/rostock_laage_airport_4k.dds");

	// ファイルパス
	std::string filePath[2] = { "resources/uvChecker.png", "resources/monsterBall.png" };

	spriteCommon_ = new SpriteCommon();
	// スプライト共通部の初期化
	spriteCommon_->Initialize(dxBase_);

	// スプライトの初期化
	sprite_ = new Sprite();
	sprite_->Initialize(spriteCommon_, "resources/uvChecker.png");

	// カメラの初期化
	camera_ = new Camera();
	camera_->SetRotate({ 0.0f, 1.75f, 0.0f });
	camera_->SetTranslate({ 0.0f, 0.0f, 0.0f });

	// ライトマネージャの初期化
	lightManager_ = LightManager::GetInstance();
	lightManager_->Initialize(dxBase_);

	// モデルマネージャー
	modelManager_ = ModelManager::GetInstance();
	// 3Dモデルマネージャの初期化
	modelManager_->Initialize(dxBase_);

	object3dCommon_ = new Object3dCommon();
	// 3Dオブジェクト共通部の初期化
	object3dCommon_->Initialize(dxBase_);

	// .objファイルからモデルを読み込む
	modelManager_->LoadModel("terrain.obj");

	// 3dオブジェクトの初期化
	terrain_ = new Object3d();
	terrain_->Initialize(object3dCommon_);

	// 初期化済みの3Dオブジェクトにモデルを紐づける
	terrain_->SetModel("terrain.obj");
	terrain_->SetCamera(camera_);

	// パーティクルマネージャ
	particleManager_ = ParticleManager::GetInstance();
	particleManager_->Initialize(dxBase_, srvManager_, camera_);

	// 円のパーティクルグループを作成
	particleManager_->CreateParticleGroup("circle", "resources/circle.png");

	Transform particleTransform;
	particleTransform.translate = { 0.0f, 0.0f, 0.0f };
	particleEmitter_ = new ParticleEmitter("circle", particleTransform, 30, 1.0f);

	// 球の初期化
	sphere_ = new Sphere();
	sphere_->Initialize(object3dCommon_, "resources/monsterBall.png");
	sphere_->SetCamera(camera_);

	// スカイボックス共通部の初期化
	skyboxCommon_ = new SkyboxCommon();
	skyboxCommon_->Initialize(dxBase_);

	// スカイボックスの初期化
	skybox_ = new Skybox();
	skybox_->Initialize(skyboxCommon_, "resources/rostock_laage_airport_4k.dds");
	skybox_->SetCamera(camera_);

	imGuiManager_ = new ImGuiManager();
	imGuiManager_->Initialize(winApp_, dxBase_);

	// オーディオ初期化
	audio_ = new Audio();
	audio_->Initialize();

	// 音声読み込み
	soundData1 = audio_->SoundLoadFile("resources/Alarm01.wav");
	soundData2 = audio_->SoundLoadFile("resources/The_maze_of_aqua.mp3");
	// 音声再生
	audio_->SoundPlayWave(soundData2, true);
}

void Game::Update() {
	if (winApp_->ProcessMessage()) {
		endRequest_ = true;
	}

	// キー入力の更新
	input_->Update();

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

	particleManager_->Update();

	particleEmitter_->Update();

	// 球の更新
	sphere_->Update();

	// スプライトの更新
	sprite_->Update();

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
	Vector3 skyboxPos = skybox_->GetTranslate();
	ImGui::DragFloat3("skyboxPos", &skyboxPos.x, 0.01f);
	skybox_->SetTranslate(skyboxPos);
	lightManager_->DebugPointLight();
	ImGui::End();
#endif

	imGuiManager_->End();
}

void Game::Draw() {
	// 描画前処理
	dxBase_->PreDraw();

	srvManager_->PreDraw();

	skyboxCommon_->DrawSetting();

	skybox_->Draw();

	// 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	object3dCommon_->DrawSetting();

	// 3Dオブジェクトの描画
	//terrain->Draw();

	// 球の描画
	//sphere->Draw();

	//particleManager->Draw();

	// 共通描画設定
	spriteCommon_->DrawSetting();

	// スプライトの描画
	//sprite->Draw();

	imGuiManager_->Draw();

	// 描画後処理
	dxBase_->PostDraw();
}

void Game::Finalize() {
	CloseHandle(dxBase_->GetFenceEvent());

	// キー入力処理解放
	delete input_;

	// スプライトの解放
	delete sprite_;

	// スプライト共通部の解放
	delete spriteCommon_;

	// 3dオブジェクトの解放
	delete terrain_;

	// 球の解放
	delete sphere_;

	// 3dオブジェクト共通部の解放
	delete object3dCommon_;

	// スカイボックスの解放
	delete skybox_;

	// スカイボックス共通部の初期化
	delete skyboxCommon_;

	// テクスチャマネージャの終了
	textureManager_->Finalize();

	// 3Dモデルマネージャの終了
	modelManager_->Finalize();

	// パーティクルマネージャの終了
	particleManager_->Finalize();

	// SRVマネージャの解放
	srvManager_->Finalize();

	// ImGuiマネージャの終了処理
	imGuiManager_->Finalize();

	// ImGuiマネージャの解放
	delete imGuiManager_;
	imGuiManager_ = nullptr;

	// ライトマネージャの解放
	lightManager_->Finalize();

	// DirectX解放
	delete dxBase_;

	// WindowsAPIの終了処理
	winApp_->Finalize();

	// WindowsAPI解放
	delete winApp_;
	winApp_ = nullptr;

	// XAudio2解放
	audio_->Finalize();
	// 音声データ開放
	audio_->SoundUnload(&soundData1);
	audio_->SoundUnload(&soundData2);

	// オーディオマネジャーの解放
	audio_ = nullptr;
	delete audio_;
}

