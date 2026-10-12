#include "Gun.h"
#include "MathFunction.h"
#include "ImGuiManager.h"
#include "Input.h"
#include "Object3dCommon.h"

using namespace std;
using namespace MathFunction;

void Gun::Initialize(Camera* camera, const std::string& filePath, Vector3 position) {
	object3d_ = make_unique<Object3d>();
	object3d_->Initialize();
	object3d_->SetEnvironmentMapTexture(filePath);
	object3d_->SetTranslate(position);

	audio_ = Audio::GetInstance();

	camera_ = camera;

	filePath_ = filePath;
}

Gun::~Gun() {
	for (Bullet* bullet : bullets_) {
		delete bullet;
		bullet = nullptr;
	}

	audio_->SoundStopWave(gunSound_);
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
	posNear = TransformNormal(posNear, matInverseVP);
	posFar = TransformNormal(posFar, matInverseVP);

	// マウスレイの方向
	Vector3 mouseDirection = posFar - posNear;
	mouseDirection = Normalize(mouseDirection);

	bulletVelocity_ = mouseDirection * bulletSpeed_ * kDeltaTime_;

	bullets_.remove_if([](Bullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			bullet = nullptr;
			return true;
		}
		return false;
		});

	if (Input::GetInstance()->TriggerKey(DIK_Z)) {
		Bullet* bullet = new Bullet();
		bullet->Initialize(camera_, filePath_, object3d_->GetTranslate(), bulletVelocity_);
		bullets_.push_back(bullet);

		gunSound_ = audio_->SoundPlayWave("gunShoot.mp3", false, 1.0f);
	}

	for (Bullet* bullet : bullets_) {
		bullet->Update();
	}
}

void Gun::DebugUpdate() {
#ifdef USE_IMGUI
	if (ImGui::CollapsingHeader("Bullet")) {
		for (Bullet* bullet : bullets_) {
			bullet->DebugUpdate();
		}
	}
#endif
}

void Gun::Draw() {
	Object3dCommon::GetInstance()->DrawSetting();
	object3d_->Draw();

	for (Bullet* bullet : bullets_) {
		bullet->Draw();
	}
}
