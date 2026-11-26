#include "Enemy.h"
#include "Gun.h"
#include "MathFunction.h"
#include "Object3dCommon.h"
#ifdef CREATE_HITBOX
#include "LineCommon.h"
#endif

using namespace MathFunction;

void Enemy::Initialize(Gun* gun, Camera* camera, Vector3 position) {
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize();
	object3d_->SetScale({ width_, height_, depth_ });
	object3d_->SetTranslate(position);
	object3d_->SetModel("uvCube.obj");
	position_ = position;

	gun_ = gun;
	camera_ = camera;

	aabb_.min = { object3d_->GetTranslate().x - width_, object3d_->GetTranslate().y - height_, object3d_->GetTranslate().z - depth_ };
	aabb_.max = { object3d_->GetTranslate().x + width_, object3d_->GetTranslate().y + height_, object3d_->GetTranslate().z + depth_ };

#ifdef CREATE_HITBOX
	hitBox_ = std::make_unique<HitBox>();
	hitBox_->Initialize(camera_);
#endif

	emitterTransform_.scale = { 0.05f, 0.05f, 1.0f };
	emitterTransform_.rotate = { 0.0f, 0.0f, 0.0f };
	emitterTransform_.translate = { 0.0f, 0.0f, 0.0f };
	Vector3 particleVelocity = { 0.0f, 0.0f, 0.0f }; // 動かない
	Vector4 particleColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	particleEmitter_ = std::make_unique<ParticleEmitter>("shotEffect", emitterTransform_, particleVelocity, particleColor, 1.0f, 1, 2.0f);
}

void Enemy::Update() {
	switch (state_) {
	case State::kApproach:
		if (stopTime_ <= 0) {
			position_.z -= speed_ * kDeltaTime_;
			object3d_->SetTranslate(position_);
		} else {
			stopTime_--;
		}

		StartAttack();

		break;
	case State::kAttack:
		break;

	case State::kDead:
		break;
	}

	aabb_.min = { object3d_->GetTranslate().x - width_, object3d_->GetTranslate().y - height_, object3d_->GetTranslate().z - depth_ };
	aabb_.max = { object3d_->GetTranslate().x + width_, object3d_->GetTranslate().y + height_, object3d_->GetTranslate().z + depth_ };

	object3d_->Update();

#ifdef CREATE_HITBOX
	hitBox_->Update(aabb_);
#endif
}

void Enemy::Draw() {
	Object3dCommon::GetInstance()->DrawSetting();
	object3d_->Draw();

#ifdef CREATE_HITBOX
	LineCommon::GetInstance()->DrawSetting();
	hitBox_->Draw();
#endif
}

void Enemy::OnCollision() {
	hp_--;

	if (hp_ <= 0) {
		isDead_ = true;
	}

	stopTime_ = 30;
	particleEmitter_->SetTranslate({ object3d_->GetTranslate().x, object3d_->GetTranslate().y, object3d_->GetTranslate().z - depth_ * 2 });
	particleEmitter_->Emit();
}

void Enemy::StartAttack() {
	if (!gun_) {
		return;
	}

	if (Length(object3d_->GetTranslate() - gun_->GetTranslate()) <= 0.5f) {
		state_ = State::kAttack;
		gun_->OnCollision();
		isVanished_ = true;
	}
}
