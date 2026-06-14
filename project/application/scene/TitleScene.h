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
#include "Audio.h"
#include "Sphere.h"
#include "BaseScene.h"

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

	// パーティクルマネージャ
	ParticleManager* particleManager_ = nullptr;

	// パーティクルエミッター
	ParticleEmitter* particleEmitter_ = nullptr;

	// オーディオ
	Audio* audio_ = nullptr;

	// サウンドデータ1
	Audio::SoundData soundData1;

	// シーンマネージャ
	SceneManager* sceneManager_ = nullptr;
};
