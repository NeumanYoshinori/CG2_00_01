#include "Bullet.h"
#include "MathFunction.h"

using namespace std;
using namespace MathFunction;

void Bullet::Initialize(const std::string& filePath, Primitive* primitive, Vector3 position, Vector3 velocity) {
	object3d_ = make_unique<Object3d>();
	object3d_->Initialize();
	object3d_->SetEnvironmentMapTexture(filePath);
	object3d_->SetPrimitive(primitive);

	position_ = position;
	object3d_->SetTranslate(position_);
	velocity_ = velocity;
}

void Bullet::Update() {
	position_ += velocity_;
	object3d_->SetTranslate(position_);

	object3d_->Update();

	// 時間経過でデス
	if (--deathTimer_ <= 0) {
		isDead_ = true;
	}
}

void Bullet::Draw() {
	object3d_->Draw();
}

void Bullet::DebugUpdate() {
	object3d_->DebugUpdate();
}
