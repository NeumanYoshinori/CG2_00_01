#pragma once
#include <string>
#include <assimp/scene.h>
#include "DirectXBase.h"
#include "MathFunction.h"
#include <map>
#include "Transform.h"
#include "SrvManager.h"

// 3Dモデル
class Model {
public:
	// ノード
	struct Node {
		QuaternionTransform transform{};
		Matrix4x4 localMatrix{};
		std::string name{};
		std::vector<Node> children;
	};

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
		Matrix4x4 uvTransform;
		int32_t enableLighting;
		float shininess;
		float environmentCoefficient;
		float alphaReference;
		int32_t useEnvironmentMap;
		float padding[3];
	};

	// マテリアルデータ
	struct MaterialData {
		std::string textureFilePath;
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

	// 初期化
	void Initialize(const std::string& directoryPath, const std::string& filename);

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

	Animation GetAnimation() const { return animation_; }

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

	// objファイルのデータ
	ModelData modelData_;

	DirectXBase* dxBase_ = nullptr;

	// アニメーション
	Animation animation_{};

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

