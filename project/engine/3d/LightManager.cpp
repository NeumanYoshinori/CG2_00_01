#include "LightManager.h"
#include <externals/imgui/imgui.h>
#include "MathFunction.h"
#include <numbers>
#include "DirectXBase.h"

using namespace std;
using namespace Microsoft::WRL;
using namespace MathFunction;

unique_ptr<LightManager> LightManager::instance_ = nullptr;

LightManager* LightManager::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = make_unique<LightManager>(ConstructorKey());
	}
	return instance_.get();
}

void LightManager::Finalize() {
	instance_.reset();
}

void LightManager::DebugLight() {
#ifdef USE_IMGUI
	// 平行光源
	ImGui::ColorEdit3("directionalLightColor", &constMap_->directionalLights_[0].color.x);
	ImGui::DragFloat3("directionalLightDirection", &constMap_->directionalLights_[0].direction.x, 0.01f);
	ImGui::DragFloat("directionalLightIntensity", &constMap_->directionalLights_[0].intensity, 0.1f);

	// ポイントライト1
	ImGui::ColorEdit3("pointLightColor1", &constMap_->pointLights_[0].color.x);
	ImGui::DragFloat3("pointLightPos1", &constMap_->pointLights_[0].position.x, 0.01f);
	ImGui::DragFloat("pointLightIntensity1", &constMap_->pointLights_[0].intensity, 0.01f);
	ImGui::DragFloat("pointLightRadius1", &constMap_->pointLights_[0].radius, 0.01f);
	ImGui::DragFloat("pointLightDecay1", &constMap_->pointLights_[0].decay, 0.01f);
	bool pointLightActive1 = constMap_->pointLights_[0].isActive;
	if (ImGui::Checkbox("pointLightActive1", &pointLightActive1)) {
		constMap_->pointLights_[0].isActive = pointLightActive1;
	}

	// ポイントライト2
	ImGui::ColorEdit3("pointLightColor2", &constMap_->pointLights_[1].color.x);
	ImGui::DragFloat3("pointLightPos2", &constMap_->pointLights_[1].position.x, 0.01f);
	ImGui::DragFloat("pointLightIntensity2", &constMap_->pointLights_[1].intensity, 0.01f);
	ImGui::DragFloat("pointLightRadius2", &constMap_->pointLights_[1].radius, 0.01f);
	ImGui::DragFloat("pointLightDecay2", &constMap_->pointLights_[1].decay, 0.01f);
	bool pointLightActive2 = constMap_->pointLights_[1].isActive;
	if (ImGui::Checkbox("pointLightActive2", &pointLightActive2)) {
		constMap_->pointLights_[1].isActive = pointLightActive2;
	}

	// スポットライト1
	ImGui::ColorEdit3("spotLightColor1", &constMap_->spotLights_[0].color.x);
	ImGui::DragFloat3("spotLightPos1", &constMap_->spotLights_[0].position.x, 0.01f);
	ImGui::DragFloat("spotLightIntensity1", &constMap_->spotLights_[0].intensity, 0.01f);
	ImGui::DragFloat3("spotLightDirection1", &constMap_->spotLights_[0].direction.x, 0.01f);
	ImGui::DragFloat("spotLightDistance1", &constMap_->spotLights_[0].distance, 0.01f);
	ImGui::DragFloat("spotLightDecay1", &constMap_->spotLights_[0].decay, 0.01f);
	ImGui::DragFloat("spotLightCosAngle1", &constMap_->spotLights_[0].cosAngle, 0.01f);
	ImGui::DragFloat("spotLightCosFalloffStart", &constMap_->spotLights_[0].cosFalloffStart, 0.01f);
	bool spotLightActive = constMap_->spotLights_[0].isActive;
	if (ImGui::Checkbox("spotLightActive1", &spotLightActive)) {
		constMap_->spotLights_[0].isActive = spotLightActive;
	}
#endif
}

void LightManager::Initialize() {
	constBuff_ = DirectXBase::GetInstance()->CreateBufferResource(sizeof(ConstBufferData));
	constBuff_->Map(0, nullptr, reinterpret_cast<void**>(&constMap_));

	for (int i = 0; i < kMaxDirectionalLights_; i++) {
		constMap_->directionalLights_[i].color = { 1.0f, 1.0f, 1.0f, 1.0f };
		constMap_->directionalLights_[i].direction = Normalize({ 0.0f, -1.0f, 0.0f });
		constMap_->directionalLights_[i].intensity = 0.0f;
	}

	for (int i = 0; i < kMaxPointLights_; i ++) {
		constMap_->pointLights_[i].color = { 1.0f, 1.0f, 1.0f, 1.0f };
		constMap_->pointLights_[i].position = { 0.0f, 0.0f, 0.0f };
		constMap_->pointLights_[i].intensity = 1.0f;
		constMap_->pointLights_[i].radius = 2.0f;
		constMap_->pointLights_[i].decay = 0.8f;
		constMap_->pointLights_[i].isActive = false;
	}

	for (int i = 0; i < kMaxSpotLights_; i++) {
		constMap_->spotLights_[i].color = { 1.0f, 1.0f, 1.0f, 1.0f };
		constMap_->spotLights_[i].position = { 2.0f, 1.25f, 0.0f };
		constMap_->spotLights_[i].distance = 7.0f;
		constMap_->spotLights_[i].direction = Normalize({ -1.0f, -1.0f, 0.0f });
		constMap_->spotLights_[i].intensity = 1.0f;
		constMap_->spotLights_[i].decay = 2.0f;
		constMap_->spotLights_[i].cosAngle = cos(numbers::pi_v<float> / 3.0f);
		constMap_->spotLights_[i].cosFalloffStart = 1.0f;
		constMap_->spotLights_[i].isActive = true;
	}
}

void LightManager::Draw() {
	ComPtr<ID3D12GraphicsCommandList> commandList = DirectXBase::GetInstance()->GetCommandList();

	commandList->SetGraphicsRootConstantBufferView(3, constBuff_->GetGPUVirtualAddress());
}
