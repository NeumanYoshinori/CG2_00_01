#include "Enemy.h"
#include "Gun.h"
#include "MathFunction.h"
#include "Object3dCommon.h"

using namespace MathFunction;

Enemy::Enemy(Camera* camera, const std::string& filePath, Vector3 position) {
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize();
	object3d_->SetEnvironmentMapTexture(filePath);
	object3d_->SetTranslate(position);
	object3d_->SetModel("uvCube.obj");
	position_ = position;

	aabb_.min = { object3d_->GetTranslate().x - width_ / 2.0f, object3d_->GetTranslate().y - height_ / 2.0f, object3d_->GetTranslate().z - depth_ / 2.0f };
	aabb_.max = { object3d_->GetTranslate().x + width_ / 2.0f, object3d_->GetTranslate().y + height_ / 2.0f, object3d_->GetTranslate().z + depth_ / 2.0f };
}

void Enemy::Update() {
	switch (state_) {
	case State::kApproach:
		position_.z -= speed_ * kDeltaTime_;
		object3d_->SetTranslate(position_);

		break;
	case State::kAttack:
		break;

	case State::kDead:
		break;
	}

	object3d_->Update();

	StartAttack();
}

void Enemy::Draw() {
	Object3dCommon::GetInstance()->DrawSetting();
	object3d_->Draw();
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
	if (state_ == State::kAttack) {
		return;
	}

	if (Length(object3d_->GetTranslate() - gun_->GetPosition()) <= 10.0f) {
		state_ = State::kAttack;
	}
}
