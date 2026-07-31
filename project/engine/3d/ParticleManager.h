#pragma once
#include "DirectXBase.h"
#include "SrvManager.h"
#include "MathFunction.h"
#include "Transform.h"
#include <unordered_map>
#include "Camera.h"
#include "Primitive.h"
#include <random>
#include "Model.h"

class ParticleManager {
public:
	struct AccelerationField {
		Vector3 acceleration;
		AABB area;
	};

	// シングルトンインスタンスの取得
	static ParticleManager* GetInstance();

	// 終了
	void Finalize();

	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Draw();

	// パーティクルグループの生成
	void CreateParticleGroup(const std::string type, const std::string name, const std::string textureFilePath, int32_t flipX, int32_t flipY);

	// パーティクルの生成
	void Emit(const std::string name, const Vector3& size, const Vector3& angle, const Vector3& position, const Vector3& velocity, const Vector4& color, float lifeTime, uint32_t count);

	// カメラをセット
	void SetCamera(Camera* camera) { camera_ = camera; }

	// namespace省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	// コンストラクタに渡すための鍵
	class ConstructorKey {
	private:
		ConstructorKey() = default;
		friend class ParticleManager;
	};

	// PassKeyを受け取るコンストラクタ
	explicit ParticleManager(ConstructorKey) {}

private:
	struct Particle {
		Transform transform;
		Vector3 velocity;
		Vector4 color;
		float lifeTime;
		float currentTime;
	};

	struct ParticleForGPU {
		Matrix4x4 WVP;
		Matrix4x4 World;
		Vector4 color;
		int32_t flipX;
		int32_t flipY;
	};

	struct ParticleGroup {
		std::list<Particle> particles;
		uint32_t srvIndex = 0;
		ComPtr<ID3D12Resource> instancingResource;
		uint32_t numInstance = 0;
		ParticleForGPU* instancingData = nullptr;
		int32_t flipX = false;
		int32_t flipY = false;
		Model* model = nullptr;
		std::unique_ptr<Primitive> primitive;
	};

	// ルートシグネチャの作成
	void CreateRootSignature();

	// グラフィックスパイプラインの生成
	void GenerateGraphicsPipeline();

	// パーティクル生成関数
	Particle MakeNewParticle(std::mt19937& randomEngine, const Vector3& scale, const Vector3& rotate, const Vector3& translate, const Vector3& velocity, const Vector4& color, float lifeTime);

	// インスタンス
	static std::unique_ptr<ParticleManager> instance_;

	// DirectXBase
	DirectXBase* dxBase_ = nullptr;

	// SRVマネジャー
	SrvManager* srvManager_ = nullptr;

	// 乱数生成器
	std::random_device seedGenerator_;
	std::mt19937 randomEngine_;

	// ルートシグネチャ
	ComPtr<ID3D12RootSignature> rootSignature_;

	ComPtr<ID3D12PipelineState> graphicsPipelineState_;

	// パーティクルグループコンテナ
	std::unordered_map<std::string, ParticleGroup> particleGroups_;
	
	// 最大インスタンス数
	const uint32_t kNumMaxInstance_ = 100;

	// カメラ
	Camera* camera_ = nullptr;

	~ParticleManager() = default;
	ParticleManager(ParticleManager&) = delete;
	ParticleManager& operator=(ParticleManager&) = delete;

	// default_delete にアクセスを許可する
	friend struct std::default_delete<ParticleManager>;
};


