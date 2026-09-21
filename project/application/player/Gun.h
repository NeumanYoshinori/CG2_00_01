#pragma once
#include "Bullet.h"
#include "Input.h"
#include "Audio.h"
#include "camera.h"

class Gun {
public:
	~Gun();

	// 初期化
	void Initialize(Camera* camera, const std::string& filePath, Primitive* primitive, Vector3 position);

	// 更新
	void Update(POINT mousePosition);

	// デバッグ
	void DebugUpdate();

	// 描画
	void Draw();

private:
	std::unique_ptr<Object3d> object3d_;

	Primitive* primitive_ = nullptr;

	std::list<Bullet*> bullets_;

	Input* input_ = nullptr;

	Audio* audio_ = nullptr;

	Camera* camera_ = nullptr;

	std::string filePath_;

	Vector3 bulletVelocity_{};

	float bulletSpeed_ = 5.0f;

	IXAudio2SourceVoice* gunSound_ = nullptr;
};

