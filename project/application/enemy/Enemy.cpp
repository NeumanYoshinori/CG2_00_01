#include "Enemy.h"
#include "Gun.h"
#include "MathFunction.h"
#include "Object3dCommon.h"
#ifdef CREATE_HITBOX
#include "LineCommon.h"
#endif

using namespace MathFunction;

void Enemy::Initialize(Gun* gun, Camera* camera, const std::string& filePath, Vector3 position) {
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize();
	object3d_->SetEnvironmentMapTexture(filePath);
	object3d_->SetTranslate(position);
	object3d_->SetRotate({ 0.0f, 3.14f, 0.0f });
	object3d_->SetModel("sneakWalk.gltf");
	position_ = position;

	gun_ = gun;
	camera_ = camera;

	aabb_.min = { object3d_->GetTranslate().x - halfWidth_, object3d_->GetTranslate().y - halfHeight_, object3d_->GetTranslate().z - halfDepth_ };
	aabb_.max = { object3d_->GetTranslate().x + halfWidth_, object3d_->GetTranslate().y + height_ * 2, object3d_->GetTranslate().z + halfDepth_ };

#ifdef CREATE_HITBOX
	hitBox_ = std::make_unique<HitBox>();
	hitBox_->Initialize(width_, height_, depth_, camera_);
#endif
}

void Enemy::Update() {
	switch (state_) {
	case State::kApproach:
		position_.z -= speed_ * kDeltaTime_;
		object3d_->SetTranslate(position_);

		StartAttack();

		break;
	case State::kAttack:
		break;

	case State::kDead:
		break;
	}

	aabb_.min = { object3d_->GetTranslate().x - halfWidth_, object3d_->GetTranslate().y - halfHeight_, object3d_->GetTranslate().z - halfDepth_ };
	aabb_.max = { object3d_->GetTranslate().x + halfWidth_, object3d_->GetTranslate().y + height_ * 2, object3d_->GetTranslate().z + halfDepth_ };

	object3d_->Update();

#ifdef CREATE_HITBOX
	hitBox_->SetTranslate({ position_.x, position_.y + height_, position_.z });
	hitBox_->Update();
#endif
}

void Enemy::Draw() {
	Object3dCommon::GetInstance()->SkinningDrawSetting();
	object3d_->Draw();

#ifdef CREATE_HITBOX
	LineCommon::GetInstance()->DrawSetting();
	hitBox_->Draw();
#endif
}

void Enemy::OnCollision() {
	if (isDead_) {
		return;
	}

	hp_--;

	if (hp_ <= 0) {
		isDead_ = true;
	}
}

void Enemy::StartAttack() {
	if (Length(object3d_->GetTranslate() - gun_->GetTranslate()) <= 10.0f) {
		state_ = State::kAttack;
	}
}
