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
	textureManager_->LoadTexture("GameOver.png");
	textureManager_->LoadTexture("GameClear.png");
	textureManager_->LoadTexture("hud_character_0.png");
	textureManager_->LoadTexture("hud_character_1.png");
	textureManager_->LoadTexture("hud_character_2.png");
	textureManager_->LoadTexture("hud_character_3.png");
	textureManager_->LoadTexture("hud_character_4.png");
	textureManager_->LoadTexture("hud_character_5.png");
	textureManager_->LoadTexture("hud_character_6.png");
	textureManager_->LoadTexture("hud_character_7.png");
	textureManager_->LoadTexture("hud_character_8.png");
	textureManager_->LoadTexture("hud_character_9.png");
	textureManager_->LoadTexture("circle2.png");

	// カメラの初期化
	camera_ = make_unique<Camera>();
	camera_->SetTranslate({ 0.0f, 0.2f, 0.0f });

	// モデルマネージャのインスタンス取得
	modelManager_ = ModelManager::GetInstance();
	modelManager_->LoadModel("uvCube.obj");
	modelManager_->CreatePrimitive("shotEffect", "Plane", "circle2.png");
	modelManager_->LoadModel("skydomeSpace.obj");
	modelManager_->LoadModel("rockyTrale.obj");

	// 3Dオブジェクト基盤部分のインスタンス取得
	object3dCommon_ = Object3dCommon::GetInstance();
	object3dCommon_->SetDefaultCamera(camera_.get());

	spriteCommon_ = SpriteCommon::GetInstance();

	// 地面の初期化
	terrain_ = make_unique<Object3d>();
	terrain_->Initialize();
	terrain_->SetTranslate({ 0.0f, 0.0f, 10.0f });
	terrain_->SetModel("rockyTrale.obj");

	// 銃の初期化
	gun_ = make_unique<Gun>();
	gun_->Initialize(camera_.get(), camera_->GetTranslate());

	// レティクルの初期化
	crosshair_ = make_unique<Sprite>();
	crosshair_->Initialize("crosshair138.png");
	crosshair_->SetAnchorPoint({ 0.5f, 0.5f });

	// ImGuiマネージャの初期化
	imGuiManager_ = ImGuiManager::GetInstance();

	// オーディオの初期化
	audio_ = Audio::GetInstance();
	audio_->SoundLoadFile("gunShoot.mp3");

	// 乱数生成器の初期化
	randomEngine_ = mt19937(seedGenerator_());

	postEffect_ = PostEffect::GetInstance();

	shoot_ = make_unique<Sprite>();
	shoot_->Initialize("Shoot.png");

	particleManager_ = ParticleManager::GetInstance();
	particleManager_->SetCamera(camera_.get());
	particleManager_->CreateParticleGroup("Primitive", "shotEffect", "shotEffect", false, false);

	skydome_ = make_unique<Object3d>();
	skydome_->Initialize();
	skydome_->SetModel("skydomeSpace.obj");

	gameOver_ = make_unique<Sprite>();
	gameOver_->Initialize("GameOver.png");

	gameClear_ = make_unique<Sprite>();
	gameClear_->Initialize("GameClear.png");

	for (int32_t i = 0; i < 10; ++i) {
		numArray_[i] = make_unique<Sprite>();
	}

	numArray_[0]->Initialize("hud_character_0.png");
	numArray_[1]->Initialize("hud_character_1.png");
	numArray_[2]->Initialize("hud_character_2.png");
	numArray_[3]->Initialize("hud_character_3.png");
	numArray_[4]->Initialize("hud_character_4.png");
	numArray_[5]->Initialize("hud_character_5.png");
	numArray_[6]->Initialize("hud_character_6.png");
	numArray_[7]->Initialize("hud_character_7.png");
	numArray_[8]->Initialize("hud_character_8.png");
	numArray_[9]->Initialize("hud_character_9.png");
}

void GamePlayScene::Finalize() {
	for (Enemy* enemy : enemies_) {
		delete enemy;
		enemy = nullptr;
	}

	particleManager_->ClearParticles("shotEffect");

	audio_->SoundUnload("gunShoot.mp3");
}

void GamePlayScene::Update() {
	// カメラの更新
	camera_->Update();

	skydome_->Update();

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
		PostEffect::GetInstance()->SetColor({ clamp(counter_ / duration_, 0.0f, 1.0f), clamp(counter_ / duration_, 0.0f, 1.0f), clamp(counter_ / duration_, 0.0f, 1.0f) });
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
		// 銃の更新
		gun_->Update(mousePosition_);
		gun_->GetEnemys(enemies_);

		if (gun_->GetHP() == 0) {
			phase_ = Phase::kGameOver;
		}

		particleManager_->Update();

		for (Enemy* enemy : enemies_) {
			if (enemy->IsDead()) {
				enemyCount--;
			}
		}

		enemies_.remove_if([](Enemy* enemy) {
			if (enemy->IsDead() || enemy->IsVanished()) {
				delete enemy;
				enemy = nullptr;
				return true;
			}
			return false;
			});

		if (enemySpawnTimer_ > 0) {
			enemySpawnTimer_--;
		} else if (enemySpawnTimer_ <= 0 && enemyCount < kMaxEnemyCount) {
			uniform_real_distribution<float> distribution(-0.2f, 0.2f);
			Enemy* enemy = new Enemy();
			enemy->Initialize(gun_.get(), camera_.get(), Vector3(distribution(randomEngine_), 0.2f, 5.0f));
			enemies_.push_back(enemy);
			enemySpawnTimer_ = enemySpawnFrequency_;
			enemyCount++;
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

		score_ = gun_->GetScore();

		if (score_ >= 20) {
			phase_ = Phase::kGameClear;
			scoreArray_[0] = score_ / 10;
			score_ %= 10;
			scoreArray_[1] = score_ / 1;
			score_ %= 1;
			numArray_[scoreArray_[0]]->SetPosition({ 600.0f, 490.0f });
			numArray_[scoreArray_[1]]->SetPosition({ 640.0f, 490.0f });
		}

		if (gun_->GetHP() == 0) {
			phase_ = Phase::kGameOver;
			scoreArray_[0] = score_ / 10;
			score_ %= 10;
			scoreArray_[1] = score_ / 1;
			score_ %= 1;
			numArray_[scoreArray_[0]]->SetPosition({ 600.0f, 490.0f });
			numArray_[scoreArray_[1]]->SetPosition({ 640.0f, 490.0f });
		}

		break;
	case Phase::kGameClear:
		// SPACEキーを押したら
		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			counter_ = 0.0f;
			phase_ = Phase::KFadeOut;
			PostEffect::GetInstance()->SetPostEffect(PostEffect::PostEffectType::FullScreen);
			PostEffect::GetInstance()->SetColor({ 1.0f, 1.0f, 1.0f });
		}

		gameClear_->Update();

		for (int32_t i = 0; i < 2; ++i) {
			numArray_[scoreArray_[i]]->Update();
		}

		break;
	case Phase::kGameOver:
		// SPACEキーを押したら
		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			counter_ = 0.0f;
			phase_ = Phase::KFadeOut;
			PostEffect::GetInstance()->SetPostEffect(PostEffect::PostEffectType::FullScreen);
			PostEffect::GetInstance()->SetColor({ 1.0f, 1.0f, 1.0f });
		}

		gameOver_->Update();

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

	if (ImGui::CollapsingHeader("Terrain")) {
		terrain_->DebugUpdate();
	}

	ImGui::End();
#endif

	// ImGui受付終了
	imGuiManager_->End();
}

void GamePlayScene::Draw() {
	// SRVマネージャの描画前処理
	SrvManager::GetInstance()->PreDraw();

	object3dCommon_->DrawSetting();

	skydome_->Draw();

	terrain_->Draw();

	particleManager_->Draw();

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
	case Phase::kGameClear:
		spriteCommon_->GetInstance()->DrawSetting();
		gameClear_->Draw();

		for (int32_t i = 0; i < 2; ++i) {
			numArray_[scoreArray_[i]]->Draw();
		}

		break;
	case Phase::kGameOver:
		spriteCommon_->GetInstance()->DrawSetting();
		gameOver_->Draw();

		for (int32_t i = 0; i < 2; ++i) {
			numArray_[scoreArray_[i]]->Draw();
		}

		break;
	case Phase::KFadeOut:
		break;
	}
}

void GamePlayScene::ImGuiDraw() {
	// ImGuiの描画
	imGuiManager_->Draw();
}
