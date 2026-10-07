#pragma once
#include "Bullet.h"
#include "Audio.h"
#include "camera.h"

class Gun {
public:
	Gun(Camera* camera, const std::string& filePath, Vector3 position);

	~Gun();

	// 更新
	void Update(POINT mousePosition);

	// デバッグ
	void DebugUpdate();

	// 描画
	void Draw();

	std::list<Bullet*> GetBullets() { return bullets_; }

	AABB GetAABB() { return aabb_; }

	Vector3 GetPosition() { return object3d_->GetTranslate(); }

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

	AABB aabb_{};

	float width_ = 1.0f;
	float height_ = 1.0f;
	float depth_ = 1.0f;
};

