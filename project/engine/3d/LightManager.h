#pragma once
#include "DirectXBase.h"
#include "SrvManager.h"
#include "MathFunction.h"

class LightManager {
public:
	static const uint32_t kMaxDirectionalLights = 1;
	static const uint32_t kMaxPointLights = 2;
	static const uint32_t kMaxSpotLights = 1;

	// 平行光源
	struct DirectionalLight {
		Vector4 color;
		Vector3 direction;
		float intensity;
	};

	// ポイントライト
	struct PointLight {
		Vector4 color;
		Vector3 position;
		float intensity;
		float radius;
		float decay;
		int32_t isActive;
		float padding;
	};

	// スポットライト
	struct SpotLight {
		Vector4 color;
		Vector3 position;
		float intensity;
		Vector3 direction;
		float distance;
		float decay;
		float cosAngle;
		float cosFalloffStart;
		int32_t isActive;
		float padding[3];
	};

	struct ConstBufferData {
		std::array<DirectionalLight, kMaxDirectionalLights> directionalLights_;
		std::array<PointLight, kMaxPointLights> pointLights_;
		std::array<SpotLight, kMaxSpotLights> spotLights_;
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

	DirectXBase* dxBase_ = nullptr;

	SrvManager* srvManager_ = nullptr;

	LightManager() = default;
	~LightManager() = default;
	LightManager(LightManager&) = delete;
	LightManager& operator=(LightManager&) = delete;
};

