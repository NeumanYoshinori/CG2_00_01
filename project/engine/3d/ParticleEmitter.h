#pragma once
#include "Transform.h"
#include <cstdint>
#include <string>

class ParticleEmitter {
public:
	// コンストラクタ
	ParticleEmitter(std::string name, Transform transform, uint32_t count, float frequency);

	// 更新
	void Update();

	// パーティクルの発生（Emit）
	void Emit();

private:
	std::string name_;
	Transform transform_;
	uint32_t count_ = 0;
	float frequency_ = 0.0f;
	float frequencyTime_ = 0.0f;
};

