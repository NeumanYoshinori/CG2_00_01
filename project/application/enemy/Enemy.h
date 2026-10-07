#pragma once
#include "Object3d.h"

class Gun;

class Enemy {
public:
	Enemy(Camera* camera, const std::string& filePath, Vector3 position);

	void Update();

	void Draw();

	AABB GetAABB() { return aabb_; }

	void OnCollision();

	void StartAttack();

	Vector3 GetPosition() const { return position_; }

	void SetGun(Gun* gun) { gun_ = gun; }

	bool IsDead() const { return isDead_; }

private:
	enum struct State {
		kApproach,
		kAttack,
		kDead
	};

	std::unique_ptr<Object3d> object3d_;

	Vector3 position_{};

	float speed_ = 2.0f;

	const float kDeltaTime_ = 1.0f / 60.0f;

	AABB aabb_{};

	bool isCollision_ = false;

	float width_ = 1.0f;
	float height_ = 1.0f;
	float depth_ = 1.0f;

	uint32_t hp_ = 5;

	State state_ = State::kApproach;

	Gun* gun_ = nullptr;

	bool isDead_ = false;
};

