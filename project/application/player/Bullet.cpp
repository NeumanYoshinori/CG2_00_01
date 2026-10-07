#include "Bullet.h"
#include "MathFunction.h"
#include "LineCommon.h"
#include "Object3dCommon.h"

using namespace std;
using namespace MathFunction;

Bullet::Bullet(Camera* camera, const std::string& envFilePath, Vector3 position, Vector3 velocity) {
	object3d_ = make_unique<Object3d>();
	object3d_->Initialize();
	object3d_->SetEnvironmentMapTexture(envFilePath);
	object3d_->SetModel("monsterBall");

	position_ = position;
	object3d_->SetTranslate(position_);
	velocity_ = velocity;
	camera_ = camera;

	width_ = object3d_->GetScale().x;
	height_ = object3d_->GetScale().y;
	depth_ = object3d_->GetScale().z;

	aabb_.min = { object3d_->GetTranslate().x - width_, object3d_->GetTranslate().y - height_, object3d_->GetTranslate().z - depth_ };
	aabb_.max = { object3d_->GetTranslate().x + width_, object3d_->GetTranslate().y + height_, object3d_->GetTranslate().z + depth_ };

	// コライダー作成
	CreateCollider();
}

void Bullet::Update() {
	position_ += velocity_;
	object3d_->SetTranslate(position_);

	object3d_->Update();
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
	vertex[0] = { aabb_.min.x, aabb_.min.y, aabb_.min.z };
	vertex[1] = { aabb_.max.x, aabb_.min.y, aabb_.min.z };
	vertex[2] = { aabb_.min.x, aabb_.max.y, aabb_.min.z };
	vertex[3] = { aabb_.max.x, aabb_.max.y, aabb_.min.z };
	vertex[4] = { aabb_.min.x, aabb_.min.y, aabb_.max.z };
	vertex[5] = { aabb_.max.x, aabb_.min.y, aabb_.max.z };
	vertex[6] = { aabb_.min.x, aabb_.max.y, aabb_.max.z };
	vertex[7] = { aabb_.max.x, aabb_.max.y, aabb_.max.z };

	for (uint32_t i = 0; i < 8; ++i) {
		vertex[i] = TransformNormal(TransformNormal(vertex[i], camera_->GetViewProjectionMatrix()), MakeViewportMatrix(0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.1f, 1.0f));
	}

	for (uint32_t i = 0; i < 12; ++i) {
		lines_[i] = make_unique<Line>();
	}

	lines_[0]->Initialize(Vector2(vertex[0].x, vertex[0].y), Vector2(vertex[1].x, vertex[1].y), Vector4(255.0f, 165.0f, 0.0f, 1.0f));
	lines_[1]->Initialize(Vector2(vertex[0].x, vertex[0].y), Vector2(vertex[2].x, vertex[2].y), Vector4(255.0f, 165.0f, 0.0f, 1.0f));
	lines_[2]->Initialize(Vector2(vertex[0].x, vertex[0].y), Vector2(vertex[4].x, vertex[4].y), Vector4(255.0f, 165.0f, 0.0f, 1.0f));
	lines_[3]->Initialize(Vector2(vertex[1].x, vertex[1].y), Vector2(vertex[3].x, vertex[3].y), Vector4(255.0f, 165.0f, 0.0f, 1.0f));
	lines_[4]->Initialize(Vector2(vertex[1].x, vertex[1].y), Vector2(vertex[5].x, vertex[5].y), Vector4(255.0f, 165.0f, 0.0f, 1.0f));
	lines_[5]->Initialize(Vector2(vertex[2].x, vertex[2].y), Vector2(vertex[3].x, vertex[3].y), Vector4(255.0f, 165.0f, 0.0f, 1.0f));
	lines_[6]->Initialize(Vector2(vertex[2].x, vertex[2].y), Vector2(vertex[6].x, vertex[6].y), Vector4(255.0f, 165.0f, 0.0f, 1.0f));
	lines_[7]->Initialize(Vector2(vertex[3].x, vertex[3].y), Vector2(vertex[7].x, vertex[7].y), Vector4(255.0f, 165.0f, 0.0f, 1.0f));
	lines_[8]->Initialize(Vector2(vertex[4].x, vertex[4].y), Vector2(vertex[5].x, vertex[5].y), Vector4(255.0f, 165.0f, 0.0f, 1.0f));
	lines_[9]->Initialize(Vector2(vertex[4].x, vertex[4].y), Vector2(vertex[6].x, vertex[6].y), Vector4(255.0f, 165.0f, 0.0f, 1.0f));
	lines_[10]->Initialize(Vector2(vertex[5].x, vertex[5].y), Vector2(vertex[7].x, vertex[7].y), Vector4(255.0f, 165.0f, 0.0f, 1.0f));
	lines_[11]->Initialize(Vector2(vertex[6].x, vertex[6].y), Vector2(vertex[7].x, vertex[7].y), Vector4(255.0f, 165.0f, 0.0f, 1.0f));
}
