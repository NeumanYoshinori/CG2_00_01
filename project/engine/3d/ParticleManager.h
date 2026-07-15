#pragma once
#include "DirectXBase.h"
#include "SrvManager.h"
#include "MathFunction.h"
#include "Transform.h"
#include <unordered_map>
#include "Camera.h"

class ParticleManager {
public:
	struct AccelerationField {
		Vector3 acceleration;
		AABB area;
	};

	enum ParticleType {
		Plane,
		Ring,
		Cylinder,
		kNum
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
	void CreateParticleGroup(ParticleType type, const std::string name, const std::string textureFilePath);

	// パーティクルの生成
	void Emit(const std::string name, const Vector3& size, const Vector3& angle, const Vector3& position, const Vector3& velocity, const Vector4& color, uint32_t count);

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
		float alphaReference;
		float padding2;
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
		uint32_t numVertices;
	};

	struct ParticleForGPU {
		Matrix4x4 WVP;
		Matrix4x4 World;
		Vector4 color;
	};

	struct ParticleGroup {
		MaterialData materialData;
		std::list<Particle> particles;
		uint32_t srvIndex = 0;
		ComPtr<ID3D12Resource> instancingResource;
		uint32_t numInstance = 0;
		ParticleForGPU* instancingData = nullptr;
		ParticleType type{};
	};

	// ルートシグネチャの作成
	void CreateRootSignature();

	// グラフィックスパイプラインの生成
	void GenerateGraphicsPipeline();

	// planeの頂点データ作成
	void CreatePlaneVertexData();

	// ringの頂点データ作成
	void CreateRingVertexData();

	// cylinderの頂点データ作成
	void CreateCylinderVertexData();

	// マテリアルデータ作成
	void CreateMaterialData();

	// パーティクル生成関数
	Particle MakeNewParticle(const Vector3& scale, const Vector3& rotate, const Vector3& translate, const Vector3& velocity, const Vector4& color);

	// インスタンス
	static std::unique_ptr<ParticleManager> instance_;

	// DirectXBase
	DirectXBase* dxBase_ = nullptr;

	// SRVマネジャー
	SrvManager* srvManager_ = nullptr;

	// ルートシグネチャ
	ComPtr<ID3D12RootSignature> rootSignature_;

	ComPtr<ID3D12PipelineState> graphicsPipelineState_;

	// パーティクルグループコンテナ
	std::unordered_map<std::string, ParticleGroup> particleGroups_;
	
	// 最大インスタンス数
	const uint32_t kNumMaxInstance_ = 100;

	// 頂点リソース
	ComPtr<ID3D12Resource> vertexResource_;

	VertexData* vertexData_ = nullptr;
	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	ComPtr<ID3D12Resource> indexResource_;
	uint32_t* indexData_ = nullptr;
	// インデックスバッファビューを作成する
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

	// カメラ
	Camera* camera_ = nullptr;

	// マテリアルリソース
	ComPtr<ID3D12Resource> materialResource_;
	Material* materialData_ = nullptr;

	const uint32_t kRingDivide_ = 32;

	// 頂点数
	uint32_t numVertex_ = 0;
	// インデックス数
	uint32_t numIndex_ = 6;

	const uint32_t kCylinderDivide_ = 32;

	~ParticleManager() = default;
	ParticleManager(ParticleManager&) = delete;
	ParticleManager& operator=(ParticleManager&) = delete;

	// default_delete にアクセスを許可する
	friend struct std::default_delete<ParticleManager>;
};

