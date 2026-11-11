#pragma once
#include "Bullet.h"
#include "Audio.h"
#include "camera.h"
#include "ParticleEmitter.h"
#include <random>

class Enemy;

class Gun {
public:
	void Initialize(Camera* camera, Vector3 position);

	~Gun();

	// 更新
	void Update(POINT mousePosition);

	// デバッグ
	void DebugUpdate();

	// 描画
	void Draw();

	const std::list<Bullet*> GetBullets() { return bullets_; }

	Vector3 GetTranslate() { return object3d_->GetTranslate(); }

	void OnCollision();

	void Damaged();

	int32_t GetHP() const { return hp_; }

	int32_t GetScore() const { return score_; }

	void GetEnemys(const std::list<Enemy*> enemy) { enemy_ = enemy; }

private:
	std::unique_ptr<Object3d> object3d_;

	std::list<Bullet*> bullets_;

	Audio* audio_ = nullptr;

	Camera* camera_ = nullptr;

	std::string filePath_;

	Vector3 bulletVelocity_{};

	float bulletSpeed_ = 50.0f;

	const float kDeltaTime_ = 1.0f / 60.0f;

	IXAudio2SourceVoice* gunSound_ = nullptr;

	int32_t hp_ = 10;

	bool isHit_ = false;

	uint32_t damageTimer_ = 5;

	int32_t score_ = 0;

	std::list<Enemy*> enemy_;
};

