#include "GamePlayScene.h"
#include "Input.h"
#include "PostEffect.h"
#include "SceneManager.h"
#include "Collider.h"

using namespace std;
using namespace MathFunction;

void GamePlayScene::Initialize() {
	textureManager_ = TextureManager::GetInstance();

	// テクスチャを読み込む
	textureManager_->LoadTexture("monsterBall.png");
	textureManager_->LoadTexture("rostock_laage_airport_4k.dds");
	textureManager_->LoadTexture("crosshair138.png");
	textureManager_->LoadTexture("Shoot.png");

	// カメラの初期化
	camera_ = make_unique<Camera>();
	camera_->SetTranslate(Vector3(0.0f, 0.2f, 0.0f));

	// スカイボックス共通部の初期化
	skyboxCommon_ = SkyboxCommon::GetInstance();
	skyboxCommon_->SetDefaultCamera(camera_.get());

	// モデルマネージャのインスタンス取得
	modelManager_ = ModelManager::GetInstance();
	modelManager_->LoadModel("AnimatedCube.gltf");
	modelManager_->LoadModel("uvCube.obj");
	modelManager_->CreatePrimitive("monsterBall", "Sphere", "monsterBall.png");

	// 3Dオブジェクト基盤部分のインスタンス取得
	object3dCommon_ = Object3dCommon::GetInstance();
	object3dCommon_->SetDefaultCamera(camera_.get());

	spriteCommon_ = SpriteCommon::GetInstance();

	// 地面の初期化
	terrain_ = make_unique<Object3d>();
	terrain_->Initialize();
	terrain_->SetModel("terrain.obj");
	terrain_->SetEnvironmentMapTexture("rostock_laage_airport_4k.dds");

	// 銃の初期化
	gun_ = make_unique<Gun>();
	gun_->Initialize(camera_.get(), "rostock_laage_airport_4k.dds", camera_->GetTranslate());

	// レティクルの初期化
	crosshair_ = make_unique<Sprite>();
	crosshair_->Initialize("crosshair138.png");
	crosshair_->SetAnchorPoint({ 0.5f, 0.5f });

	// ImGuiマネージャの初期化
	imGuiManager_ = ImGuiManager::GetInstance();

	// オーディオの初期化
	audio_ = Audio::GetInstance();
	audio_->SoundLoadFile("gunShoot.mp3");
	audio_->SoundLoadFile("The_maze_of_aqua.mp3");

	// 乱数生成器の初期化
	randomEngine_ = mt19937(seedGenerator_());

	postEffect_ = PostEffect::GetInstance();

	shoot_ = make_unique<Sprite>();
	shoot_->Initialize("Shoot.png");
}

void GamePlayScene::Finalize() {
	for (Enemy* enemy : enemies_) {
		delete enemy;
		enemy = nullptr;
	}

	audio_->SoundUnload("gunShoot.mp3");

	audio_->SoundStopWave(bgm_);
	audio_->SoundUnload("The_maze_of_aqua.mp3");
}

void GamePlayScene::Update() {
	// カメラの更新
	camera_->Update();

	terrain_->Update();

	// マウス座標を取得
	GetCursorPos(&mousePosition_);

	// クライエントに変換
	ScreenToClient(WinApp::GetInstance()->GetHwnd(), &mousePosition_);

	// スプライトの座標をマウス位置に設定
	crosshair_->SetPosition({ static_cast<float>(mousePosition_.x), static_cast<float>(mousePosition_.y) });
	// スプライトの更新
	crosshair_->Update();

	const list<Bullet*> bullets = gun_->GetBullets();

	switch (phase_) {
	case Phase::kFadeIn:
		// 1フレーム分の秒数をカウントアップ
		counter_ += 1.0f / 60.0f;
		// フェード継続時間に達したら打ち止め
		counter_ = min(counter_, duration_);
		PostEffect::GetInstance()->SetColor(Vector3(clamp(counter_ / duration_, 0.0f, 1.0f), clamp(counter_ / duration_, 0.0f, 1.0f), clamp(counter_ / duration_, 0.0f, 1.0f)));
		if (counter_ >= duration_) {
			phase_ = Phase::kStart;
		}

		break;
	case Phase::kStart:
		shootTimer_--;
		if (shootTimer_ == 0) {
			phase_ = Phase::kMain;
		}

		shoot_->Update();

		break;
	case Phase::kMain:
		// Rキーを押したら
		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			counter_ = 0.0f;
			phase_ = Phase::KFadeOut;
		}

		// 銃の更新
		gun_->Update(mousePosition_);

		enemies_.remove_if([](Enemy* enemy) {
			if (enemy->IsDead()) {
				delete enemy;
				enemy = nullptr;
				return true;
			}
			return false;
			});

		if (enemySpawnTimer_ > 0) {
			enemySpawnTimer_--;
		}
		else {
			uniform_real_distribution<float> distribution(-4.0f, 4.0f);
			Enemy* enemy = new Enemy();
			enemy->Initialize(gun_.get(), camera_.get(), "rostock_laage_airport_4k.dds", Vector3(distribution(randomEngine_), 0.0f, 20.0f));
			enemies_.push_back(enemy);
			enemySpawnTimer_ = enemySpawnFrequency_;
		}

		// 敵の更新
		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

		for (Bullet* bullet : bullets) {
			for (Enemy* enemy : enemies_) {
				Collider::CheckBulletEnemyCollision(bullet, enemy);
			}
		}

		break;
	case Phase::KFadeOut:
		// 1フレーム分の秒数をカウントアップ
		counter_ += 1.0f / 60.0f;
		// フェード継続時間に達したら打ち止め
		counter_ = min(counter_, duration_);
		postEffect_->SetColor(Vector3(clamp(1.0f - counter_ / duration_, 0.0f, 1.0f), clamp(1.0f - counter_ / duration_, 0.0f, 1.0f), clamp(1.0f - counter_ / duration_, 0.0f, 1.0f)));

		if (counter_ >= duration_) {
			// シーン切り替え
			SceneManager::GetInstance()->ChangeScene("TITLE");
		}
	}

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

	switch (phase_) {
	case Phase::kFadeIn:
		break;
	case Phase::kStart:
		SpriteCommon::GetInstance()->DrawSetting();
		shoot_->Draw();

		break;
	case Phase::kMain:
		for (Enemy* enemy : enemies_) {
			// 敵の描画
			enemy->Draw();
		}

		// 銃の描画
		gun_->Draw();

		// スプライト描画前処理
		spriteCommon_->GetInstance()->DrawSetting();

		// スプライトの描画
		crosshair_->Draw();

		break;
	case Phase::KFadeOut:
		break;
	}
}

void GamePlayScene::ImGuiDraw() {
	// ImGuiの描画
	imGuiManager_->Draw();
}
