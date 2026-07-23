#pragma once
#include "Input.h"
#include "TextureManager.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "ModelManager.h"
#include "ParticleManager.h"
#include "ParticleEmitter.h"
#include "ImGuiManager.h"
#include "Audio.h"
#include "Sphere.h"
#include "SkyboxCommon.h"
#include "Skybox.h"
#include "BaseScene.h"
#include <random>

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

	// 地面
	std::unique_ptr<Object3d> terrain_;

	// 球
	std::unique_ptr<Sphere> sphere_;

	std::random_device seedGenerator_;
	std::mt19937 randomEngine_;

	Transform planeTransform{};

	// パーティクルマネージャ
	ParticleManager* particleManager_ = nullptr;

	// パーティクルエミッター
	std::unique_ptr<ParticleEmitter> planeEmitter_;
	std::unique_ptr<ParticleEmitter> cylinderEmitter_;

	// スカイボックス共通部
	SkyboxCommon* skyboxCommon_ = nullptr;

	// スカイボックス
	std::unique_ptr<Skybox> skybox_;

	// ImGuiマネジャー
	ImGuiManager* imGuiManager_ = nullptr;

	// オーディオ
	Audio* audio_ = nullptr;

	// サウンドデータ2
	Audio::SoundData soundData2;

	IXAudio2SourceVoice* bgmVoice_ = nullptr;

	float rgb[3] = { 1.0f, 1.0f, 1.0f };

	bool useSepia = false;
};

