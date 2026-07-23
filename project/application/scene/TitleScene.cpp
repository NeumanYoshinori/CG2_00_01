#include "TitleScene.h"
#include "SceneManager.h"

using namespace std;

void TitleScene::Initialize() {
	// インスタンス取得
	input_ = Input::GetInstance();

	textureManager_ = TextureManager::GetInstance();

	// テクスチャを読み込む
	textureManager_->LoadTexture("resources/uvChecker.png");
	textureManager_->LoadTexture("resources/rostock_laage_airport_4k.dds");

	// スプライトの初期化
	sprite_ = make_unique<Sprite>();
	sprite_->Initialize("resources/uvChecker.png");

	// オーディオの初期化
	audio_ = Audio::GetInstance();
	// 音声読み込み
	soundData1 = audio_->SoundLoadFile("resources/audios/Alarm01.wav");

	// 音声再生
	bgmVoice_ = audio_->SoundPlayWave(soundData1, true);

	imGuiManager_ = ImGuiManager::GetInstance();
}

void TitleScene::Finalize() {
	audio_->SoundStopWave(bgmVoice_);

	// 音声データ開放
	audio_->SoundUnload(&soundData1);
}

void TitleScene::Update() {
	// スプライトの更新
	sprite_->Update();

	// ENTERキーを押したら
	if (input_->TriggerKey(DIK_R)) {
		// シーン切り替え
		SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
	}

	imGuiManager_->Begin();
#ifdef USE_IMGUI
	// スプライトのImGuiの開始
	ImGui::Begin("Sprite");

	// 座標
	Vector2 spritePos = sprite_->GetPosition();
	ImGui::DragFloat2("Po", &spritePos.x, 0.1f);
	sprite_->SetPosition(spritePos);

	// 角度
	float spriteRot = sprite_->GetRotation();
	ImGui::DragFloat("Rot", &spriteRot, 0.1f);
	sprite_->SetRotation(spriteRot);

	// アンカーポイント
	Vector2 anchorPoint = sprite_->GetAnchorPoint();
	ImGui::DragFloat2("AnchorPoint", &anchorPoint.x, 0.1f);
	sprite_->SetAnchorPoint(anchorPoint);

	// x座標フリップ
	bool flipX = sprite_->IsFlipX();
	ImGui::Checkbox("flipX", &flipX);
	sprite_->SetFlipX(flipX);

	// Y座標フリップ
	bool flipY = sprite_->IsFlipY();
	ImGui::Checkbox("flipY", &flipY);
	sprite_->SetFlipY(flipY);

	// テクスチャ左上座標
	Vector2 textureLeftTop = sprite_->GetTextureLeftTop();
	ImGui::DragFloat2("TextureLeftTop", &textureLeftTop.x, 0.1f);
	sprite_->SetTextureLeftTop(textureLeftTop);

	// テクスチャのサイズ
	Vector2 textureSize = sprite_->GetTextureSize();
	ImGui::DragFloat2("TextureSize", &textureSize.x, 0.1f);
	sprite_->SetTextureSize(textureSize);
	ImGui::End();
#endif
	ImGuiManager::GetInstance()->End();
}

void TitleScene::Draw() {
	// SRVマネージャ描画前処理
	SrvManager::GetInstance()->PreDraw();

	// 共通描画設定
	SpriteCommon::GetInstance()->DrawSetting();

	// スプライトの描画
	sprite_->Draw();
}

void TitleScene::ImGuiDraw() {
	// ImGui受付開始
	ImGuiManager::GetInstance()->Draw();
}
