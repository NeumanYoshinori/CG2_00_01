#pragma once
#include "Object3d.h"
#ifdef CREATE_COLLIDER
#include "Line.h"
#endif

class Bullet {
public:
	void Initialize(Camera* camera, const std::string& envFilePath, Vector3 position, Vector3 velocity);

	void Update();

	void Draw();

	void DebugUpdate();

	bool IsDead() const { return isDead_; }

	AABB GetAABB() const { return aabb_; }

	// 衝突時
	void OnCollision();

	// コライダー作成
	void CreateCollider();

private:
#ifdef CREATE_COLLIDER
	std::array<std::unique_ptr<Line>, 12> lines_;
#endif

	// Object3d
	std::unique_ptr<Object3d> object3d_;

	bool isDead_ = false;

	Vector3 velocity_{};

	Vector3 position_{};

	AABB aabb_{};

	float width_ = 1.0f;
	float height_ = 1.0f;
	float depth_ = 1.0f;
	float halfWidth_ = width_ / 2.0f;
	float halfHeight_ = height_ / 2.0f;
	float halfDepth_ = depth_ / 2.0f;

	// 寿命
	static const int32_t kLifeTime_ = 60 * 2;

	// デスタイマー
	int32_t deathTimer_ = kLifeTime_;

	Camera* camera_ = nullptr;
};

