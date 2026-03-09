#pragma once
#include "DirectXBase.h"
#include "SrvManager.h"
#include "MathFunction.h"
#include "Transform.h"
#include <unordered_map>
#include "Camera.h"
#include <random>

class ParticleManager {
public:
	// 頂点データ
	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};

	// マテリアルデータ
	struct Material {
		Vector4 color;
		int32_t enableLighting;
		float padding[3];
		Matrix4x4 uvTransform;
	};

	// マテリアルデータ
	struct MaterialData {
		std::string textureFilePath;
		uint32_t textureIndex = 0;
	};

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
	};

	struct Emitter {
		Transform transform; // !< エミッタのTransform
		uint32_t count; // !< 発生数
		float frequency; // !< 発生頻度
		float frequencyTime; // !< 頻度用時刻
	};

	struct AccelerationField {
		Vector3 acceleration;
		AABB area;
	};

	struct ParticleGroup {
		MaterialData materialData;
		std::list<Particle> particles;
		uint32_t srvIndex;
		Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource;
		uint32_t numInstance;
		ParticleForGPU* instancingData;
	};

	// シングルトンインスタンスの取得
	static ParticleManager* GetInstance();

	// 終了
	void Finalize();

	// 初期化
	void Initialize(DirectXBase* dxBase, SrvManager* srvManager, Camera* camera);

	// 更新
	void Update();

	// 描画
	void Draw();

	// パーティクルグループの生成
	void CreateParticleGroup(const std::string name, const std::string textureFilePath);

	// パーティクルの生成
	void Emit(const std::string name, const Vector3& position, uint32_t count);

private:
	// ルートシグネチャの作成
	void CreateRootSignature();

	// グラフィックスパイプラインの生成
	void GenerateGraphicsPipeline();

	// 頂点データ作成
	void CreateVertexData();

	// マテリアルデータ作成
	void CreateMaterialData();

	// インスタンス
	static ParticleManager* instance;

	// DirectXBase
	DirectXBase* dxBase_ = nullptr;

	// SRVマネジャー
	SrvManager* srvManager_ = nullptr;

	std::random_device seedGenerator_;
	std::mt19937 randomEngine_;

	// ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;

	// パーティクルグループコンテナ
	std::unordered_map<std::string, ParticleGroup> particleGroups;

	const uint32_t kNumMaxInstance = 100;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;

	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};

	// カメラ
	Camera* camera_ = nullptr;

	// マテリアルリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = nullptr;
	Material* materialData = nullptr;

	ParticleManager() = default;
	~ParticleManager() = default;
	ParticleManager(ParticleManager&) = delete;
	ParticleManager& operator=(ParticleManager&) = delete;
};

