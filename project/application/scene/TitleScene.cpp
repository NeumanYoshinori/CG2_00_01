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

	// 共通描画設定
	spriteCommon_->DrawSetting();

	// スプライトの描画
	sprite_->Draw();
}

void TitleScene::ImGuiDraw() {
	return;
}
