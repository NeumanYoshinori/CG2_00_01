#pragma once
#include "Input.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "TextureManager.h"
#include "Audio.h"
#include "BaseScene.h"
#include "ImGuiManager.h"
#include "SkyboxCommon.h"
#include "Skybox.h"
#include "Camera.h"

// 前方宣言
class SceneManager;

// ゲームプレイシーン
class TitleScene : public BaseScene {
public:
	// 初期化
	void Initialize() override;

	// 終了
	void Finalize() override;

	// 毎フレーム更新
	void Update() override;

	// 描画
	void Draw() override;

	// ImGui描画
	void ImGuiDraw() override;

private:
	// WindowsAPI
	WinApp* winApp_ = nullptr;

	// DirectX基盤部分
	DirectXBase* dxBase_ = nullptr;

	// 入力
	Input* input_ = nullptr;

	// テクスチャマネージャ
	TextureManager* textureManager_ = nullptr;

	// スプライト
	std::unique_ptr<Sprite> sprite_;

	// オーディオ
	Audio* audio_ = nullptr;

	IXAudio2SourceVoice* bgmVoice_ = nullptr;

	SkyboxCommon* skyboxCommon_ = nullptr;
	std::unique_ptr<Skybox> skybox_;

	std::unique_ptr<Camera> camera_;

	SpriteCommon* spriteCommon_ = nullptr;
	std::unique_ptr<Sprite> title_;

	// ImGuiマネージャ
	ImGuiManager* imGuiManager_ = nullptr;
};
