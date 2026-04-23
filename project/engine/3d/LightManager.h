#pragma once
#include "PointLight.h"
#include "SpotLight.h"
#include "DirectXBase.h"
#include "SrvManager.h"

class LightManager {
public:
	static const uint32_t kMaxPointLights = 2;
	static const uint32_t kMaxSpotLights = 1;

	struct ConstBufferData {
		std::array<PointLight::ConstBufferData, kMaxPointLights> pointLights_;
		std::array<SpotLight::ConstBufferData, kMaxSpotLights> spotLights_;
	};

	static LightManager* GetInstance();

	void Initialize(DirectXBase* dxBase);

	void Draw();

	void Finalize();

	void DebugPointLight();

private:
	static LightManager* instance;

	Microsoft::WRL::ComPtr<ID3D12Resource> constBuff_;
	ConstBufferData* constMap_ = nullptr;

	std::array<PointLight, kMaxPointLights> pointLights_;

	DirectXBase* dxBase_ = nullptr;

	SrvManager* srvManager_ = nullptr;

	float cosAngle[kMaxSpotLights] = { 0.0f };

	LightManager() = default;
	~LightManager() = default;
	LightManager(LightManager&) = delete;
	LightManager& operator=(LightManager&) = delete;
};

