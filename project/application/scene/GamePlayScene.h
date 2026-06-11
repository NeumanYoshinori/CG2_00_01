#pragma once
#include "Input.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "TextureManager.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "ModelManager.h"
#include "SrvManager.h"
#include "ParticleManager.h"
#include "ParticleEmitter.h"
#include "ImGuiManager.h"
#include "Audio.h"
#include "Sphere.h"
#include "SkyboxCommon.h"
#include "Skybox.h"

// ゲームプレイシーン
class GamePlayScene {
public:
	// 初期化
	void Initialize();

	// 終了
	void Finalize();

	// 毎フレーム更新
	void Update();

	// 描画
	void Draw();

private:
	// WindowsAPI
	WinApp* winApp_ = nullptr;

	// DirectX基盤部分
	DirectXBase* dxBase_ = nullptr;

	// 入力
	Input* input_ = nullptr;

	// SRVマネージャ
	SrvManager* srvManager_ = nullptr;

	// テクスチャマネージャ
	TextureManager* textureManager_ = nullptr;

	// スプライト共通部
	SpriteCommon* spriteCommon_ = nullptr;

	// スプライト
	Sprite* sprite_ = nullptr;

	// カメラ
	Camera* camera_ = nullptr;

	// モデルマネージャ
	ModelManager* modelManager_ = nullptr;

	// オブジェクト3D共通部
	Object3dCommon* object3dCommon_ = nullptr;

	// 地面
	Object3d* terrain_ = nullptr;

	// パーティクルマネージャ
	ParticleManager* particleManager_ = nullptr;

	// パーティクルエミッター
	ParticleEmitter* particleEmitter_ = nullptr;

	// 球
	Sphere* sphere_ = nullptr;

	// スカイボックス共通部
	SkyboxCommon* skyboxCommon_ = nullptr;

	// スカイボックス
	Skybox* skybox_ = nullptr;

	// ImGuiマネジャー
	ImGuiManager* imGuiManager_ = nullptr;

	// オーディオ
	Audio* audio_ = nullptr;

	Audio::SoundData soundData1;
	Audio::SoundData soundData2;
};

