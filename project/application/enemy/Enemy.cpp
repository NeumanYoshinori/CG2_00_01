#include "Enemy.h"
#include "Gun.h"
#include "MathFunction.h"
#include "Object3dCommon.h"
#ifdef CREATE_COLLIDER
#include "LineCommon.h"
#endif

using namespace MathFunction;

Enemy::~Enemy() {
	for (Line* line : skeleton_) {
		delete line;
		line = nullptr;
	}
}

void Enemy::Initialize(Gun* gun, Camera* camera, const std::string& filePath, Vector3 position) {
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize();
	object3d_->SetEnvironmentMapTexture(filePath);
	object3d_->SetTranslate(position);
	object3d_->SetModel("uvCube.obj");
	position_ = position;

	gun_ = gun;
	camera_ = camera;

	aabb_.min = { object3d_->GetTranslate().x - halfWidth_, object3d_->GetTranslate().y - halfHeight_, object3d_->GetTranslate().z - halfDepth_ };
	aabb_.max = { object3d_->GetTranslate().x + halfWidth_, object3d_->GetTranslate().y + halfHeight_, object3d_->GetTranslate().z + halfDepth_ };

	CreateCollider();
	CreateSkeletonLine(object3d_->GetSkeleton());
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
	aabb_.max = { object3d_->GetTranslate().x + halfWidth_, object3d_->GetTranslate().y + halfHeight_, object3d_->GetTranslate().z + halfDepth_ };

	object3d_->Update();

#ifdef CREATE_COLLIDER
	for (uint32_t i = 0; i < 12; ++i) {
		lines_[i]->SetTranslate(object3d_->GetTranslate());
		lines_[i]->Update();
	}
#endif

	const auto& joints = object3d_->GetSkeleton().joints;
	size_t lineIndex = 0;

	for (const Model::Joint& joint : joints) {
		if (!joint.parent) {
			continue;
		}

		const Model::Joint& parent = object3d_->GetSkeleton().joints[*joint.parent];

		Vector3 start = GetWorldJointPosition(parent);
		Vector3 end = GetWorldJointPosition(joint);

		if (lineIndex < skeleton_.size()) {
			skeleton_[lineIndex]->SetStart(start);
			skeleton_[lineIndex]->SetEnd(end);
			++lineIndex;
		}
	}

	for (Line* line : skeleton_) {
		line->SetTranslate({ 0.0f, 0.0f, 0.0f });
		line->SetRotate({ 0.0f, 0.0f, 0.0f });
		line->SetScale({ 1.0f, 1.0f, 1.0f });
		line->Update();
	}
}

void Enemy::Draw() {
	Object3dCommon::GetInstance()->DrawSetting();
	object3d_->Draw();

#ifdef CREATE_COLLIDER
	LineCommon::GetInstance()->DrawSetting();
	for (uint32_t i = 0; i < 12; ++i) {
		lines_[i]->Draw();
	}
#endif

	for (Line* line : skeleton_) {
		line->Draw();
	}
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

void Enemy::CreateCollider() {
#ifdef CREATE_COLLIDER
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
		lines_[i] = std::make_unique<Line>();
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
#endif
}

void Enemy::CreateSkeletonLine(Model::Skeleton skeleton) {
	for (Model::Joint& joint : skeleton.joints) {
		if (!joint.parent) {
			continue;
		}

		const Model::Joint& parent = skeleton.joints[*joint.parent];

		Vector3 start = GetWorldJointPosition(parent);
		Vector3 end = GetWorldJointPosition(joint);

		Line* line = new Line();
		line->Initialize(start, end, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		line->SetCamera(camera_);
		skeleton_.push_back(line);
	}
}

Vector3 Enemy::GetWorldJointPosition(const Model::Joint& joint) {
	Vector3 localPosition =
		object3d_->GetModel()->GetJointPosition(joint);

	Matrix4x4 worldMatrix = MakeAffineMatrix(
		object3d_->GetScale(),
		object3d_->GetRotate(),
		object3d_->GetTranslate()
	);

	Vector3 worldPosition = TransformNormal(localPosition, worldMatrix);
	return worldPosition;
}
