#include "TitleScene.h"
#include "SceneManager.h"

void TitleScene::Initialize() {
	// インスタンス取得
	dxBase_ = DirectXBase::GetInstance();

	input_ = Input::GetInstance();

	srvManager_ = SrvManager::GetInstance();

	textureManager_ = TextureManager::GetInstance();

	// テクスチャを読み込む
	textureManager_->LoadTexture("resources/uvChecker.png");

	spriteCommon_ = SpriteCommon::GetInstance();
	// スプライトの初期化
	sprite_ = new Sprite();
	sprite_->Initialize(spriteCommon_, "resources/uvChecker.png");

	// カメラの初期化
	camera_ = new Camera();
	camera_->SetRotate({ 0.0f, 1.75f, 0.0f });
	camera_->SetTranslate({ 0.0f, 0.0f, 0.0f });

	// パーティクルマネージャ
	particleManager_ = ParticleManager::GetInstance();
	particleManager_->Initialize(dxBase_, srvManager_, camera_);

	// 円のパーティクルグループを作成
	particleManager_->CreateParticleGroup("circle", "resources/circle.png");

	// パーティクルエミッターの初期化
	Transform particleTransform;
	particleTransform.translate = { 0.0f, 0.0f, 0.0f };
	particleEmitter_ = new ParticleEmitter("circle", particleTransform, 30, 1.0f);

	// オーディオの初期化
	audio_ = Audio::GetInstance();
	audio_->Initialize();
	// 音声読み込み
	soundData1 = audio_->SoundLoadFile("resources/Alarm01.wav");
	// 音声再生
	audio_->SoundPlayWave(soundData1, true);

	// シーンマネージャのインスタンス取得
	sceneManager_ = SceneManager::GetInstance();
}

void TitleScene::Finalize() {
	// スプライトの解放
	delete sprite_;
	sprite_ = nullptr;

	// パーティクルエミッターの解放
	delete particleEmitter_;
	particleEmitter_ = nullptr;

	// パーティクルマネージャの終了
	particleManager_->Finalize();

	// カメラの解放
	delete camera_;
	camera_ = nullptr;

	// オーディオの終了
	audio_->Release();

	// 音声データ開放
	audio_->SoundUnload(&soundData1);
}

void TitleScene::Update() {
	// 0キーを押したときコンソールにHit 0と表示する
	if (input_->ReleaseKey(DIK_0)) {
		OutputDebugStringA("Hit 0\n");
	}

	// カメラの更新
	camera_->Update();

	// パーティクルマネージャの更新
	particleManager_->Update();

	// パーティクルエミッターの更新
	particleEmitter_->Update();

	// スプライトの更新
	sprite_->Update();

	// ENTERキーを押したら
	if (input_->TriggerKey(DIK_RETURN)) {
		// シーン切り替え
		SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
	}
}

void TitleScene::Draw() {
	// SRVマネージャ描画前処理
	srvManager_->PreDraw();

	// パーティクルマネージャ描画
	particleManager_->Draw();

	// 共通描画設定
	spriteCommon_->DrawSetting();

	// スプライトの描画
	sprite_->Draw();
}
