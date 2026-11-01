#pragma once
#include <string>
#include <assimp/scene.h>
#include "DirectXBase.h"
#include "MathFunction.h"
#include <map>
#include "Transform.h"
#include <span>
#include "SrvManager.h"

// 3Dモデル
class Model {
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
		float shininess;
		float environmentCoefficient;
		float alphaReference;
		float padding2;
	};

	// マテリアルデータ
	struct MaterialData {
		std::string textureFilePath;
	};

	// ノード
	struct Node {
		QuaternionTransform transform{};
		Matrix4x4 localMatrix{};
		std::string name{};
		std::vector<Node> children;
	};

	struct VertexWeightData {
		float weight;
		uint32_t vertexIndex;
	};

	struct JointWeightData {
		Matrix4x4 inverseBindPoseMatrix{};
		std::vector<VertexWeightData> vertexWeights;
	};

	// モデルデータ
	struct ModelData {
		std::map<std::string, JointWeightData> skinClusterData;
		std::vector<VertexData> vertices;
		std::vector<int32_t> indices;
		MaterialData material;
		Node rootNode;
	};

	template <typename tValue>
	struct Keyframe {
		float time; // キーフレームの時刻（単位は秒）
		tValue value; // キーフレームの値
	};
	using KeyframeVector3 = Keyframe<Vector3>;
	using KeyframeQuaternion = Keyframe<Quaternion>;

	template<typename tValue>
	struct AnimationCurve {
		std::vector<Keyframe<tValue>> keyframes;
	};

	static const uint32_t kNumMaxInfluence = 4;
	struct VertexInfluence {
		std::array<float, kNumMaxInfluence> weights;
		std::array<int32_t, kNumMaxInfluence> jointIndices;
	};

	struct WellForGPU {
		Matrix4x4 skeletonSpaceMatrix; // 位置用
		Matrix4x4 skeletonSpaceInverseTransposeMatrix; // 法線用
	};

public:
	struct NodeAnimation {
		AnimationCurve<Vector3> translate;
		AnimationCurve<Quaternion> rotate;
		AnimationCurve<Vector3> scale;
	};

	struct Animation {
		float duration = 0.0f; // アニメーション全体の尺（単位は秒）
		// NodeAnimationの集合。Node名でひけるようにしておく
		std::map<std::string, NodeAnimation> nodeAnimations;
	};

	struct Joint {
		QuaternionTransform transform{}; // Transform情報
		Matrix4x4 localMatrix{}; // localMatrix
		Matrix4x4 skeletonSpaceMatrix{}; // skeletonSpaceでの変換行列
		std::string name; // 名前
		std::vector<int32_t> children; // 子jointのIndexのリスト。いなければ空
		int32_t index = 0; // 自身のIndex
		std::optional<int32_t> parent; // 親JointのIndex。いなければnull
	};

	struct Skeleton {
		int32_t root = 0; // RootJointのIndex
		std::map<std::string, int32_t> jointMap{}; // Joint名とIndexとの辞書
		std::vector<Joint> joints; // 所属しているジョイント
	};

	struct SkinCluster {
		std::vector<Matrix4x4> inverseBindPoseMatrices;
		Microsoft::WRL::ComPtr<ID3D12Resource> influenceResource;
		D3D12_VERTEX_BUFFER_VIEW influenceBufferView{};
		std::span<VertexInfluence> mappedInfluence;
		Microsoft::WRL::ComPtr<ID3D12Resource> paletteResource;
		std::span<WellForGPU> mappedPalette;
		uint32_t srvIndex = 0;
	};

	// 初期化
	void Initialize(const std::string& directoryPath, const std::string& filename);

	// Skeletonの更新
	void Update(Skeleton& skeleton);

	// SkinClusterの更新
	void Update(SkinCluster& skinCluster, const Skeleton& skeleton);

	// 描画
	void Draw();

	const ModelData& GetModelData() { return modelData_; }

	float GetEnvironmentCoefficient() { return materialData_->environmentCoefficient; }
	void SetEnvironmentCoefficient(float environmentCoefficient) { materialData_->environmentCoefficient = environmentCoefficient; }

	void SetNumInstance(uint32_t numInstance) { numInstance_ = numInstance; }

	// localMatrixをセット
	void SetLocalMatrix(Matrix4x4 localMatrix) { modelData_.rootNode.localMatrix = localMatrix; }

	// 任意の時刻の値を取得する
	Vector3 CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time);
	Quaternion CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time);

	// Animation適用
	void ApplyAnimation(Skeleton& skeleton, const Animation& animation, float animationTime);

	Animation GetAnimation() const { return animation_; }
	Skeleton GetSkeleton() const { return skeleton_; }
	SkinCluster GetSkinCluster() const { return skinCluster_; }

private:
	// 頂点データ作成
	void CreateVertexData();

	// マテリアルデータ作成
	void CreateMaterialData();

	// .objファイルの読み取り
	static ModelData LoadModelFile(const std::string& directoryPath, const std::string& filename);

	// ノードを読む
	static Node ReadNode(aiNode* node);

	// アニメーションを読み込む
	Animation LoadAnimationFile(const std::string& directoryPath, const std::string& filename);

	// skeleton作成
	Skeleton CreateSkeleton();

	// Joint作成
	int32_t CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints);

	// SkinCluster作成
	SkinCluster CreateSkinCluster();

	// objファイルのデータ
	ModelData modelData_;

	DirectXBase* dxBase_ = nullptr;

	// アニメーション
	Animation animation_{};
	// スケルトン
	Skeleton skeleton_{};
	// スキンクラスター
	SkinCluster skinCluster_{};

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_; // 頂点リソース
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData_ = nullptr;
	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_; // インデックスリソース
	// バッファリソース内のデータを指すポインタ
	uint32_t* indexData_ = nullptr;
	// バッファリソースの使い道を補足するバッファビュー
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_; // マテリアルリソース
	// バッファリソース内のデータを指すポインタ
	Material* materialData_ = nullptr;

	// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;

	uint32_t numInstance_ = 1;

	// SRVマネージャ
	SrvManager* srvManager_ = nullptr;
};

