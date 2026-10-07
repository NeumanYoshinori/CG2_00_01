#pragma once
#include "Object3d.h"
#include "Line.h"

class Bullet {
public:
	Bullet(Camera* camera, const std::string& envFilePath, Vector3 position, Vector3 velocity);

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
	std::array<std::unique_ptr<Line>, 12> lines_;

	// Object3d
	std::unique_ptr<Object3d> object3d_;

	bool isDead_ = false;

	Vector3 velocity_{};

	Vector3 position_{};

	AABB aabb_{};

	float width_ = 0.0f;
	float height_ = 0.0f;
	float depth_ = 0.0f;

	Camera* camera_ = nullptr;
};

