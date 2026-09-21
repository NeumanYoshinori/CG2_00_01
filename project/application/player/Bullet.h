#pragma once
#include "Object3d.h"
#include "Primitive.h"

class Bullet {
public:
	void Initialize(const std::string& filePath, Primitive* primitive, Vector3 position, Vector3 velocity);

	void Update();

	void Draw();

	void DebugUpdate();

	bool IsDead() const { return isDead_; }

private:
	// Object3d
	std::unique_ptr<Object3d> object3d_;

	// 寿命
	static const int32_t kLifeTime = 60 * 5;

	// デスタイマー
	int32_t deathTimer_ = kLifeTime;

	bool isDead_ = false;

	Vector3 velocity_{};

	Vector3 position_{};
};

