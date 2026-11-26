#pragma once
#include "Object3d.h"
#ifdef CREATE_HITBOX
#include "HitBox.h"
#endif
#include "ParticleEmitter.h"

class Gun;

class Enemy {
public:
	void Initialize(Gun* gun, Camera* camera, Vector3 position);

	void Update();

	void Draw();

	AABB GetAABB() { return aabb_; }

	void OnCollision();

	void StartAttack();

	Vector3 GetTranslate() const { return object3d_->GetTranslate(); }

	bool IsDead() const { return isDead_; }

	bool IsVanished() const { return isVanished_; }

private:
	enum struct State {
		kApproach,
		kAttack,
		kDead
	};

	std::unique_ptr<Object3d> object3d_;

	Vector3 position_{};

	float speed_ = 1.0f;

	const float kDeltaTime_ = 1.0f / 60.0f;

	AABB aabb_{};

	bool isCollision_ = false;

	float width_ = 0.1f;
	float height_ = 0.1f;
	float depth_ = 0.1f;
	float halfWidth_ = width_ / 2.0f;
	float halfHeight_ = height_ / 2.0f;
	float halfDepth_ = depth_ / 2.0f;

	uint32_t hp_ = 5;

	State state_ = State::kApproach;

	Gun* gun_ = nullptr;

	bool isDead_ = false;

	bool isVanished_ = false;

	Camera* camera_ = nullptr;

	bool isHit_ = false;

	uint32_t stopTime_ = 30;

#ifdef CREATE_HITBOX
	std::unique_ptr<HitBox> hitBox_;
#endif

	std::unique_ptr<ParticleEmitter> particleEmitter_;

	EulerTransform emitterTransform_{};
};

