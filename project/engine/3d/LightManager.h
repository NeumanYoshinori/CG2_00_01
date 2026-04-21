#pragma once
#include "PointLight.h"
#include "DirectXBase.h"
#include "SrvManager.h"

class LightManager {
public:
	static const uint32_t kMaxPointLights = 1;

	struct ConstBufferData {
		std::array<PointLight::ConstBufferData, kMaxPointLights> pointLights_;
	};

	static LightManager* GetInstance();

	void Initialize(DirectXBase* dxBase);

	void Update();

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

	LightManager() = default;
	~LightManager() = default;
	LightManager(LightManager&) = delete;
	LightManager& operator=(LightManager&) = delete;
};

