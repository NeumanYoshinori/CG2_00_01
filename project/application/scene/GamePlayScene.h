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
#include "BaseScene.h"

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

	// カメラ
	Camera* camera_ = nullptr;

	// モデルマネージャ
	ModelManager* modelManager_ = nullptr;

	// オブジェクト3D共通部
	Object3dCommon* object3dCommon_ = nullptr;

	// 地面
	Object3d* terrain_ = nullptr;

	// 球
	Sphere* sphere_ = nullptr;

	// パーティクルマネージャ
	ParticleManager* particleManager_ = nullptr;

	// パーティクルエミッター
	ParticleEmitter* particleEmitter_ = nullptr;

	// スカイボックス共通部
	SkyboxCommon* skyboxCommon_ = nullptr;

	// スカイボックス
	Skybox* skybox_ = nullptr;

	// ImGuiマネジャー
	ImGuiManager* imGuiManager_ = nullptr;

	// オーディオ
	Audio* audio_ = nullptr;

	// サウンドデータ2
	Audio::SoundData soundData2;
};

