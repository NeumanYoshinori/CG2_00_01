#include "HitBox.h"

using namespace MathFunction;

void HitBox::Initialize(Camera* camera) {
	for (size_t i = 0; i < kNumLines; ++i) {
		lines_[i] = std::make_unique<Line>();
		lines_[i]->SetCamera(camera);
		lines_[i]->Initialize({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 255.0f, 165.0f, 0.0f, 1.0f });
	}
}

void HitBox::Update(const AABB& aabb) {
	vertices_[0] = { aabb.min.x, aabb.min.y, aabb.min.z };
	vertices_[1] = { aabb.max.x, aabb.min.y, aabb.min.z };
	vertices_[2] = { aabb.min.x, aabb.max.y, aabb.min.z };
	vertices_[3] = { aabb.max.x, aabb.max.y, aabb.min.z };
	vertices_[4] = { aabb.min.x, aabb.min.y, aabb.max.z };
	vertices_[5] = { aabb.max.x, aabb.min.y, aabb.max.z };
	vertices_[6] = { aabb.min.x, aabb.max.y, aabb.max.z };
	vertices_[7] = { aabb.max.x, aabb.max.y, aabb.max.z };

	for (size_t i = 0; i < kNumLines; ++i) {
		lines_[i]->SetStart(vertices_[edges_[i][0]]);
		lines_[i]->SetEnd(vertices_[edges_[i][1]]);
		lines_[i]->Update();
	}
}

void HitBox::Draw() {
	for (size_t i = 0; i < kNumLines; ++i) {
		lines_[i]->Draw();
	}
}
