#pragma once
#include "MathFunction.h"
#include <memory>
#include <d3d12.h>
#include <wrl.h>
#include <array>

#pragma comment(lib, "d3d12.lib")

class LightManager {
public:
	// シングルトンインスタンスの取得
	static LightManager* GetInstance();

	// 終了
	void Finalize();

	// 初期化
	void Initialize();

	// 描画
	void Draw();

	// デバッグ
	void DebugLight();

	// コンストラクタに渡すための鍵
	class ConstructorKey {
	private:
		ConstructorKey() = default;
		friend class LightManager;
	};

	// PassKeyを受け取るコンストラクタ
	explicit LightManager(ConstructorKey) {}

private:
	static const uint32_t kMaxDirectionalLights_ = 1;
	static const uint32_t kMaxPointLights_ = 2;
	static const uint32_t kMaxSpotLights_ = 1;

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
	};

	struct ConstBufferData {
		std::array<DirectionalLight, kMaxDirectionalLights_> directionalLights_;
		std::array<PointLight, kMaxPointLights_> pointLights_;
		std::array<SpotLight, kMaxSpotLights_> spotLights_;
	};

	// シングルトンインスタンス
	static std::unique_ptr<LightManager> instance_;

	Microsoft::WRL::ComPtr<ID3D12Resource> constBuff_;
	ConstBufferData* constMap_ = nullptr;

	~LightManager() = default;
	LightManager(LightManager&) = delete;
	LightManager& operator=(LightManager&) = delete;

	// default_delete にアクセスを許可する
	friend struct std::default_delete<LightManager>;
};

