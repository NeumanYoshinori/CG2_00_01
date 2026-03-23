#include "Camera.h"
#include "WinApp.h"
#ifdef USE_IMGUI
#include "ImGuiManager.h"
#endif

using namespace MathFunction;

Camera::Camera()
	: transform({ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} })
	, fovY_(0.45f)
	, aspectRatio_(float(WinApp::kClientWidth) / float(WinApp::kClientHeight))
	, nearClip_(0.1f)
	, farClip_(100.0f)
	, worldMatrix(MakeAffineMatrix(transform.scale, transform.rotate, transform.translate))
	, viewMatrix(Inverse(worldMatrix))
	, projectionMatrix(MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_))
	, viewProjectionMatrix(Multiply(viewMatrix, projectionMatrix))
{}

void Camera::Update() {
	worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	viewMatrix = Inverse(worldMatrix);
	projectionMatrix = MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_);
	viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);
}

void Camera::DebugUpdate() {
#ifdef USE_IMGUI
	// 開発用UIの処理
	ImGui::Begin("Settings");
	ImGui::DragFloat3("CameraTranslate", &transform.translate.x, 0.01f, -10.0f, 10.0f);
	ImGui::SliderAngle("CameraRotateX", &transform.rotate.x);
	ImGui::SliderAngle("CameraRotateY", &transform.rotate.y);
	ImGui::SliderAngle("CameraRotateZ", &transform.rotate.z);
	ImGui::End();
#endif
}
