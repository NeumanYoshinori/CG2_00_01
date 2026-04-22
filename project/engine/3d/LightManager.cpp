#include "LightManager.h"
#include "ImGuiManager.h"

using namespace std;
using namespace Microsoft::WRL;

LightManager* LightManager::instance = nullptr;

LightManager* LightManager::GetInstance() {
	if (instance == nullptr) {
		instance = new LightManager;
	}
	return instance;
}

void LightManager::Finalize() {
	delete instance;
	instance = nullptr;
}

void LightManager::DebugPointLight() {
#ifdef USE_IMGUI
	ImGui::DragFloat3("pointLightPos1", &constMap_->pointLights_[0].position.x, 0.01f);
	ImGui::DragFloat("pointLightIntensity1", &constMap_->pointLights_[0].intensity, 0.01f);
	ImGui::DragFloat("pointLightRadius1", &constMap_->pointLights_[0].radius, 0.01f);
	ImGui::DragFloat("pointLightDecay1", &constMap_->pointLights_[0].decay, 0.01f);

	ImGui::DragFloat3("pointLightPos2", &constMap_->pointLights_[1].position.x, 0.01f);
	ImGui::DragFloat("pointLightIntensity2", &constMap_->pointLights_[1].intensity, 0.01f);
	ImGui::DragFloat("pointLightRadius2", &constMap_->pointLights_[1].radius, 0.01f);
	ImGui::DragFloat("pointLightDecay2", &constMap_->pointLights_[1].decay, 0.01f);
#endif
}

void LightManager::Initialize(DirectXBase* dxBase) {
	dxBase_ = dxBase;

	constBuff_ = dxBase_->CreateBufferResource(sizeof(ConstBufferData));
	constBuff_->Map(0, nullptr, reinterpret_cast<void**>(&constMap_));

	for (int i = 0; i < kMaxPointLights; i ++) {
		constMap_->pointLights_[i].position = {0.0f, 0.0f, 0.0f };
		constMap_->pointLights_[i].color = {1.0f, 1.0f, 1.0f, 1.0f};
		constMap_->pointLights_[i].intensity = 5.0f;
		constMap_->pointLights_[i].radius = 2.0f;
		constMap_->pointLights_[i].decay = 0.8f;
	}
}

void LightManager::Draw() {
	ComPtr<ID3D12GraphicsCommandList> commandList = dxBase_->GetCommandList();

	commandList->SetGraphicsRootConstantBufferView(5, constBuff_->GetGPUVirtualAddress());
}
