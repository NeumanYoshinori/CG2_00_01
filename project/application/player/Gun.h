#pragma once
#include "Bullet.h"
#include "Audio.h"
#include "camera.h"

class Enemy;

class Gun {
public:
	void Initialize(Camera* camera, const std::string& filePath, Vector3 position);

	void GetEnemys(const std::list<Enemy*> enemy) { enemy_ = enemy; }

	~Gun();

	// 更新
	void Update(POINT mousePosition);

	// デバッグ
	void DebugUpdate();

	// 描画
	void Draw();

	const std::list<Bullet*> GetBullets() { return bullets_; }

	Vector3 GetTranslate() { return object3d_->GetTranslate(); }

	bool IsDead() { return isDead_; }
	void SetIsDead(bool isDead) { isDead_ = isDead; }

	int32_t GetScore() { return score_; }

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

	bool isDead_ = false;

	int32_t score_ = 0;

	std::list<Enemy*> enemy_;
};

