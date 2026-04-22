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
	Vector3 pointLightPos1 = pointLights_[0].GetPosition();
	ImGui::DragFloat3("pointLightPos1", &pointLightPos1.x, 0.01f);
	pointLights_[0].SetPosition(pointLightPos1);
	float pointLightIntensity1 = pointLights_[0].GetIntensity();
	ImGui::DragFloat("pointLightIntensity1", &pointLightIntensity1, 0.01f);
	pointLights_[0].SetIntensity(pointLightIntensity1);
	float pointLightRadius1 = pointLights_[0].GetRadius();
	ImGui::DragFloat("pointLightRadius1", &pointLightRadius1, 0.01f);
	pointLights_[0].SetRadius(pointLightRadius1);
	float pointLightDecay1 = pointLights_[0].GetDecay();
	ImGui::DragFloat("pointLightDecay1", &pointLightDecay1, 0.01f);
	pointLights_[0].SetDecay(pointLightDecay1);

	Vector3 pointLightPos2 = pointLights_[1].GetPosition();
	ImGui::DragFloat3("pointLightPos2", &pointLightPos2.x, 0.01f);
	pointLights_[1].SetPosition(pointLightPos2);
	float pointLightIntensity2 = pointLights_[1].GetIntensity();
	ImGui::DragFloat("pointLightIntensity2", &pointLightIntensity2, 0.01f);
	pointLights_[1].SetIntensity(pointLightIntensity2);
	float pointLightRadius2 = pointLights_[1].GetRadius();
	ImGui::DragFloat("pointLightRadius2", &pointLightRadius2, 0.01f);
	pointLights_[1].SetRadius(pointLightRadius2);
	float pointLightDecay2 = pointLights_[1].GetDecay();
	ImGui::DragFloat("pointLightDecay2", &pointLightDecay2, 0.01f);
	pointLights_[1].SetDecay(pointLightDecay2);
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

		pointLights_[i].SetPosition(constMap_->pointLights_[i].position);
		pointLights_[i].SetColor(constMap_->pointLights_[i].color);
		pointLights_[i].SetIntensity(constMap_->pointLights_[i].intensity);
		pointLights_[i].SetPosition(constMap_->pointLights_[i].position);
		pointLights_[i].SetRadius(constMap_->pointLights_[i].radius);
		pointLights_[i].SetDecay(constMap_->pointLights_[i].decay);
	}
}

void LightManager::Update() {
	for (int i = 0; i < kMaxPointLights; i++) {
		constMap_->pointLights_[i].position = pointLights_[i].GetPosition();
		constMap_->pointLights_[i].color = pointLights_[i].GetColor();
		constMap_->pointLights_[i].intensity = pointLights_[i].GetIntensity();
		constMap_->pointLights_[i].radius = pointLights_[i].GetRadius();
		constMap_->pointLights_[i].decay = pointLights_[i].GetDecay();
	}
}

void LightManager::Draw() {
	ComPtr<ID3D12GraphicsCommandList> commandList = dxBase_->GetCommandList();

	commandList->SetGraphicsRootConstantBufferView(5, constBuff_->GetGPUVirtualAddress());
}
