#include "ParticleEmitter.h"
#include "ParticleManager.h"

ParticleEmitter::ParticleEmitter(std::string name, Transform transform, uint32_t count, float frequency) {
	name_ = name;
	transform_ = transform;
	count_ = count;
	frequency_ = frequency;
	frequencyTime_ = 0.0f;
}

void ParticleEmitter::Update() {
	// Δを設定
	const float kDeltaTime = 1.0f / 60.0f;
	frequencyTime_ += kDeltaTime;

	if (frequency_ <= frequencyTime_) { // 頻度より大きいなら発生
		ParticleManager::GetInstance()->Emit(name_, transform_.translate, count_); // 発生処理
		frequencyTime_ -= frequency_; // 余計に過ぎた時間も加味して頻度計算する
	}
}

void ParticleEmitter::Emit() {
	ParticleManager::GetInstance()->Emit(name_, transform_.translate, count_);
}
