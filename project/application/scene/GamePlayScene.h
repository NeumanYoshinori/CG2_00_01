#pragma once
#include "TextureManager.h"
#include "Object3dCommon.h"
#include "ModelManager.h"
#include "ParticleManager.h"
#include "ImGuiManager.h"
#include "Audio.h"
#include "SkyboxCommon.h"
#include "BaseScene.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "Gun.h"
#include "Enemy.h"
#include "LineCommon.h"

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
	// テクスチャマネージャ
	TextureManager* textureManager_ = nullptr;

	// カメラ
	std::unique_ptr<Camera> camera_;

	// モデルマネージャ
	ModelManager* modelManager_ = nullptr;

	// オブジェクト3D共通部
	Object3dCommon* object3dCommon_ = nullptr;

	// スプライト共通部
	SpriteCommon* spriteCommon_ = nullptr;

	// レティクル
	std::unique_ptr<Sprite> crosshair_;

	// パーティクルマネージャ
	ParticleManager* particleManager_ = nullptr;

	// スカイボックス共通部
	SkyboxCommon* skyboxCommon_ = nullptr;

	// ImGuiマネジャー
	ImGuiManager* imGuiManager_ = nullptr;

	// オーディオ
	Audio* audio_ = nullptr;
	IXAudio2SourceVoice* bgm_ = nullptr;

	POINT mousePosition_{};

	// 銃
	std::unique_ptr<Gun> gun_;

	// 敵
	std::list<Enemy*> enemies_;

	// 敵発生頻度
	const float enemySpawnFrequency_ = 600.0f;

	// 敵発生タイマー
	float enemySpawnTimer_ = 0;

	// 地面
	std::unique_ptr<Object3d> terrain_;

	LineCommon* lineCommon_ = nullptr;

	// 乱数生成器
	std::random_device seedGenerator_;
	std::mt19937 randomEngine_;
};

