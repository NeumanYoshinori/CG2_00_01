#pragma once
#include "Input.h"
#include "TextureManager.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "ModelManager.h"
#include "ParticleManager.h"
#include "ImGuiManager.h"
#include "Audio.h"
#include "SkyboxCommon.h"
#include "Skybox.h"
#include "BaseScene.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "Gun.h"

// ゲームプレイシーン
class GamePlayScene : public BaseScene {
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
	// 入力
	Input* input_ = nullptr;

	// テクスチャマネージャ
	TextureManager* textureManager_ = nullptr;

	// カメラ
	std::unique_ptr<Camera> camera_;

	// モデルマネージャ
	ModelManager* modelManager_ = nullptr;

	// オブジェクト3D共通部
	Object3dCommon* object3dCommon_ = nullptr;

	// 球
	std::unique_ptr<Primitive> sphere_;

	// スプライト共通部
	SpriteCommon* spriteCommon_ = nullptr;

	// レティクル
	std::unique_ptr<Sprite> crosshair_;

	// パーティクルマネージャ
	ParticleManager* particleManager_ = nullptr;

	// スカイボックス共通部
	SkyboxCommon* skyboxCommon_ = nullptr;

	// スカイボックス
	std::unique_ptr<Skybox> skybox_;

	// ImGuiマネジャー
	ImGuiManager* imGuiManager_ = nullptr;

	// オーディオ
	Audio* audio_ = nullptr;
	IXAudio2SourceVoice* bgm_;

	POINT mousePosition_{};

	// 銃
	std::unique_ptr<Gun> gun_;
};

