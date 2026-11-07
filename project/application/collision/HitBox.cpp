#include "HitBox.h"

void HitBox::Initialize(float width, float height, float depth, Camera* camera) {
	Vector3 vertex[8] = {};
	vertex[0] = { -width, -height, -depth };
	vertex[1] = { width, -height, -depth };
	vertex[2] = { -width, height, -depth };
	vertex[3] = { width, height, -depth };
	vertex[4] = { -width, -height, depth };
	vertex[5] = { width, -height, depth };
	vertex[6] = { -width, height, depth };
	vertex[7] = { width, height, depth };


	for (uint32_t i = 0; i < kNumLines; ++i) {
		lines_[i] = std::make_unique<Line>();
		lines_[i]->SetCamera(camera);
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

void HitBox::Update() {
	for (size_t i = 0; i < kNumLines; ++i) {
		lines_[i]->Update();
	}
}

void HitBox::Draw() {
	for (size_t i = 0; i < kNumLines; ++i) {
		lines_[i]->Draw();
	}
}

void HitBox::SetTranslate(Vector3 translate) {
	for (size_t i = 0; i < kNumLines; ++i) {
		lines_[i]->SetTranslate(translate);
	}
}
