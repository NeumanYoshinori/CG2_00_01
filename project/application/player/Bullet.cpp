#include "Bullet.h"
#include "MathFunction.h"
#include "LineCommon.h"
#include "Object3dCommon.h"

using namespace std;
using namespace MathFunction;

void Bullet::Initialize(Camera* camera, const std::string& envFilePath, Vector3 position, Vector3 velocity) {
	object3d_ = make_unique<Object3d>();
	object3d_->Initialize();
	object3d_->SetEnvironmentMapTexture(envFilePath);
	object3d_->SetModel("monsterBall");

	position_ = position;
	object3d_->SetTranslate(position_);
	velocity_ = velocity;
	camera_ = camera;

	aabb_.min = { object3d_->GetTranslate().x - halfWidth_, object3d_->GetTranslate().y - halfHeight_, object3d_->GetTranslate().z - halfDepth_ };
	aabb_.max = { object3d_->GetTranslate().x + halfWidth_, object3d_->GetTranslate().y + halfHeight_, object3d_->GetTranslate().z + halfDepth_ };

	// コライダー作成
	CreateCollider();
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

	for (uint32_t i = 0; i < 12; ++i) {
		lines_[i]->SetTranslate(object3d_->GetTranslate());
		lines_[i]->Update();
	}
}

void Bullet::Draw() {
	Object3dCommon::GetInstance()->DrawSetting();
	object3d_->Draw();

	LineCommon::GetInstance()->DrawSetting();
	for (uint32_t i = 0; i < 8; ++i) {
		lines_[i]->Draw();
	}
}

void Bullet::DebugUpdate() {
	object3d_->DebugUpdate();
}

void Bullet::OnCollision() {
	isDead_ = true;
}

void Bullet::CreateCollider() {
	Vector3 vertex[8] = {};
	vertex[0] = { -width_, -height_, -depth_ };
	vertex[1] = { width_, -height_, -depth_ };
	vertex[2] = { -width_, height_, -depth_ };
	vertex[3] = { width_, height_, -depth_ };
	vertex[4] = { -width_, -height_, depth_ };
	vertex[5] = { width_, -height_, depth_ };
	vertex[6] = { -width_, height_, depth_ };
	vertex[7] = { width_, height_, depth_ };

	for (uint32_t i = 0; i < 12; ++i) {
		lines_[i] = make_unique<Line>();
		lines_[i]->SetCamera(camera_);
	}

	lines_[0]->Initialize(Vector3(vertex[0].x, vertex[0].y, vertex[0].z), Vector3(vertex[1].x, vertex[1].y, vertex[1].z), Vector4(255.0f, 165.0f, 0.0f, 1.0f));
	lines_[1]->Initialize(Vector3(vertex[0].x, vertex[0].y, vertex[0].z), Vector3(vertex[2].x, vertex[2].y, vertex[2].z), Vector4(255.0f, 165.0f, 0.0f, 1.0f));
	lines_[2]->Initialize(Vector3(vertex[0].x, vertex[0].y, vertex[0].z), Vector3(vertex[4].x, vertex[4].y, vertex[4].z), Vector4(255.0f, 165.0f, 0.0f, 1.0f));
	lines_[3]->Initialize(Vector3(vertex[1].x, vertex[1].y, vertex[1].z), Vector3(vertex[3].x, vertex[3].y, vertex[3].z), Vector4(255.0f, 165.0f, 0.0f, 1.0f));
	lines_[4]->Initialize(Vector3(vertex[1].x, vertex[1].y, vertex[1].z), Vector3(vertex[5].x, vertex[5].y, vertex[5].z), Vector4(255.0f, 165.0f, 0.0f, 1.0f));
	lines_[5]->Initialize(Vector3(vertex[2].x, vertex[2].y, vertex[2].z), Vector3(vertex[3].x, vertex[3].y, vertex[3].z), Vector4(255.0f, 165.0f, 0.0f, 1.0f));
	lines_[6]->Initialize(Vector3(vertex[2].x, vertex[2].y, vertex[2].z), Vector3(vertex[6].x, vertex[6].y, vertex[6].z), Vector4(255.0f, 165.0f, 0.0f, 1.0f));
	lines_[7]->Initialize(Vector3(vertex[3].x, vertex[3].y, vertex[3].z), Vector3(vertex[7].x, vertex[7].y, vertex[7].z), Vector4(255.0f, 165.0f, 0.0f, 1.0f));
	lines_[8]->Initialize(Vector3(vertex[4].x, vertex[4].y, vertex[4].z), Vector3(vertex[5].x, vertex[5].y, vertex[5].z), Vector4(255.0f, 165.0f, 0.0f, 1.0f));
	lines_[9]->Initialize(Vector3(vertex[4].x, vertex[4].y, vertex[4].z), Vector3(vertex[6].x, vertex[6].y, vertex[6].z), Vector4(255.0f, 165.0f, 0.0f, 1.0f));
	lines_[10]->Initialize(Vector3(vertex[5].x, vertex[5].y, vertex[5].z), Vector3(vertex[7].x, vertex[7].y, vertex[7].z), Vector4(255.0f, 165.0f, 0.0f, 1.0f));
	lines_[11]->Initialize(Vector3(vertex[6].x, vertex[6].y, vertex[6].z), Vector3(vertex[7].x, vertex[7].y, vertex[7].z), Vector4(255.0f, 165.0f, 0.0f, 1.0f));
}
