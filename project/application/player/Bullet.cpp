#include "Bullet.h"
#include "MathFunction.h"
#include "Object3dCommon.h"
#ifdef CREATE_HITBOX
#include "LineCommon.h"
#endif

using namespace std;
using namespace MathFunction;

void Bullet::Initialize(Camera* camera, Vector3 position, Vector3 velocity) {
	object3d_ = make_unique<Object3d>();
	object3d_->Initialize();
	object3d_->SetScale({ width_, height_, depth_ });

	position_ = position;
	object3d_->SetTranslate(position_);
	velocity_ = velocity;
	camera_ = camera;

	aabb_.min = { object3d_->GetTranslate().x - width_, object3d_->GetTranslate().y - height_, object3d_->GetTranslate().z - depth_ };
	aabb_.max = { object3d_->GetTranslate().x + width_, object3d_->GetTranslate().y + height_, object3d_->GetTranslate().z + depth_ };

#ifdef CREATE_HITBOX
	// コライダー作成
	hitBox_ = make_unique<HitBox>();
	hitBox_->Initialize(camera_);
#endif

/*	emitterTransform_.scale = { 0.05f, 0.05f, 1.0f };
	emitterTransform_.rotate = { 0.0f, 0.0f, 0.0f };
	emitterTransform_.translate = { 0.0f, 0.0f, 0.0f };
	Vector3 particleVelocity = { 0.0f, 0.0f, 0.0f }; // 動かない
	Vector4 particleColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	particleEmitter_ = make_unique<ParticleEmitter>("shotEffec*///t", emitterTransform_, particleVelocity, particleColor, 1.0f, 1, 2.0f);
}

void Bullet::Update() {
	position_ += velocity_;
	object3d_->SetTranslate(position_);

	aabb_.min = { object3d_->GetTranslate().x - width_, object3d_->GetTranslate().y - height_, object3d_->GetTranslate().z - depth_ };
	aabb_.max = { object3d_->GetTranslate().x + width_, object3d_->GetTranslate().y + height_, object3d_->GetTranslate().z + depth_ };

	// 時間経過でデス
	if (--deathTimer_ <= 0) {
		isDead_ = true;
	}

	object3d_->Update();

#ifdef CREATE_HITBOX
	hitBox_->Update(aabb_);
#endif

	//particleEmitter_->Update();
}

void Bullet::Draw() {
	Object3dCommon::GetInstance()->DrawSetting();
	object3d_->Draw();

#ifdef CREATE_HITBOX
	LineCommon::GetInstance()->DrawSetting();
	hitBox_->Draw();
#endif
}

void Bullet::DebugUpdate() {
	object3d_->DebugUpdate();
}

void Bullet::OnCollision() {
	isDead_ = true;

	/*particleEmitter_->SetTranslate({object3d_->GetTranslate().x);
	particleEmitter_->Emit();*/
}
