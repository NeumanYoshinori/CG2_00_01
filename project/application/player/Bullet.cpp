#include "Bullet.h"
#include "MathFunction.h"
#include "Object3dCommon.h"
#ifdef CREATE_HITBOX
#include "LineCommon.h"
#endif

using namespace std;
using namespace MathFunction;

void Bullet::Initialize(Camera* camera, const std::string& envFilePath, Vector3 position, Vector3 velocity) {
	object3d_ = make_unique<Object3d>();
	object3d_->Initialize();
	object3d_->SetEnvironmentMapTexture(envFilePath);
	object3d_->SetPrimitive("monsterBall");

	position_ = position;
	object3d_->SetTranslate(position_);
	velocity_ = velocity;
	camera_ = camera;

	aabb_.min = { object3d_->GetTranslate().x - halfWidth_, object3d_->GetTranslate().y - halfHeight_, object3d_->GetTranslate().z - halfDepth_ };
	aabb_.max = { object3d_->GetTranslate().x + halfWidth_, object3d_->GetTranslate().y + halfHeight_, object3d_->GetTranslate().z + halfDepth_ };

#ifdef CREATE_HITBOX
	// コライダー作成
	hitBox_ = make_unique<HitBox>();
	hitBox_->Initialize(width_, height_, depth_, camera_);
#endif
}

void Bullet::Update() {
	position_ += velocity_;
	object3d_->SetTranslate(position_);

	aabb_.min = { object3d_->GetTranslate().x - halfWidth_, object3d_->GetTranslate().y - halfHeight_, object3d_->GetTranslate().z - halfDepth_ };
	aabb_.max = { object3d_->GetTranslate().x + halfWidth_, object3d_->GetTranslate().y + halfHeight_, object3d_->GetTranslate().z + halfDepth_ };

	// 時間経過でデス
	if (--deathTimer_ <= 0) {
		isDead_ = true;
	}

	object3d_->Update();

#ifdef CREATE_HITBOX
	for (uint32_t i = 0; i < 12; ++i) {
		hitBox_->SetTranslate(object3d_->GetTranslate());
		hitBox_->Update();
	}
#endif
}

void Bullet::Draw() {
	Object3dCommon::GetInstance()->DrawSetting();
	object3d_->Draw();

#ifdef CREATE_HITBOX
	LineCommon::GetInstance()->DrawSetting();
	for (uint32_t i = 0; i < 8; ++i) {
		hitBox_->Draw();
	}
#endif
}

void Bullet::DebugUpdate() {
	object3d_->DebugUpdate();
}

void Bullet::OnCollision() {
	isDead_ = true;
}
