#include "Gun.h"
#include "MathFunction.h"
#include "ImGuiManager.h"

using namespace std;
using namespace MathFunction;

Gun::~Gun() {
	for (Bullet* bullet : bullets_) {
		delete bullet;
		bullet = nullptr;
	}

	audio_->SoundStopWave(gunSound_);
}

void Gun::Initialize(Camera* camera, const std::string& filePath, Primitive* primitive, Vector3 position) {
	object3d_ = make_unique<Object3d>();
	object3d_->Initialize();
	object3d_->SetEnvironmentMapTexture(filePath);
	object3d_->SetTranslate(position);

	input_ = Input::GetInstance();

	audio_ = Audio::GetInstance();

	camera_ = camera;

	primitive_ = primitive;

	filePath_ = filePath;
}

void Gun::Update(POINT mousePosition) {
	object3d_->SetTranslate(camera_->GetTranslate());
	object3d_->Update();

	float ndcX = (2.0f * mousePosition.x / WinApp::kClientWidth) - 1.0f;
	float ndcY = 1.0f - (2.0f * mousePosition.y / WinApp::kClientHeight);

	// スクリーン座標
	Vector3 posNear = Vector3(ndcX, ndcY, 0);
	Vector3 posFar = Vector3(ndcX, ndcY, 1);

	Matrix4x4 matInverseVP = Inverse(camera_->GetViewProjectionMatrix());

	// スクリーン座標系からワールド座標系へ
	posNear = MathFunction::Transform(posNear, matInverseVP);
	posFar = MathFunction::Transform(posFar, matInverseVP);

	// マウスレイの方向
	Vector3 mouseDirection = posFar - posNear;
	mouseDirection = Normalize(mouseDirection);

	bulletVelocity_ = mouseDirection * bulletSpeed_;

	bullets_.remove_if([](Bullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			return true;
		}
		return false;
		});

	if (input_->TriggerKey(DIK_SPACE)) {
		Bullet* bullet = new Bullet();
		bullet->Initialize(filePath_, primitive_, object3d_->GetTranslate(), bulletVelocity_);
		bullets_.push_back(bullet);

		gunSound_ = audio_->SoundPlayWave("resources/audios/gunShoot.mp3", false, 1.0f);
	}

	for (Bullet* bullet : bullets_) {
		bullet->Update();
	}
}

void Gun::DebugUpdate() {
	if (ImGui::CollapsingHeader("Bullet")) {
		for (Bullet* bullet : bullets_) {
			bullet->DebugUpdate();
		}
	}
}

void Gun::Draw() {
	object3d_->Draw();

	for (Bullet* bullet : bullets_) {
		bullet->Draw();
	}
}
