#pragma once
#include "Object3d.h"
#ifdef CREATE_HITBOX
#include "HitBox.h"
#endif
#include "ParticleEmitter.h"

class Bullet {
public:
	void Initialize(Camera* camera, Vector3 position, Vector3 velocity);

	void Update();

	void Draw();

	void DebugUpdate();

	bool IsDead() const { return isDead_; }

	AABB GetAABB() const { return aabb_; }

	// 衝突時
	void OnCollision();

private:
#ifdef CREATE_HITBOX
	std::unique_ptr<HitBox> hitBox_;
#endif

	// Object3d
	std::unique_ptr<Object3d> object3d_;

	bool isDead_ = false;

	Vector3 velocity_{};

	Vector3 position_{};

	AABB aabb_{};

	float width_ = 0.1f;
	float height_ = 0.1f;
	float depth_ = 0.1f;

	// 寿命
	static const int32_t kLifeTime_ = 60 * 2;

	// デスタイマー
	int32_t deathTimer_ = kLifeTime_;

	Camera* camera_ = nullptr;

	/*std::unique_ptr<ParticleEmitter> particleEmitter_;

	EulerTransform emitterTransform_;*/
};

