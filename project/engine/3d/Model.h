#pragma once
#include <string>
#include <assimp/scene.h>
#include "DirectXBase.h"
#include "MathFunction.h"
#include <map>

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
		Matrix4x4 localMatrix{};
		std::string name;
		std::vector<Node> children;
	};

	// モデルデータ
	struct ModelData {
		std::vector<VertexData> vertices;
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

	struct NodeAnimation {
		AnimationCurve<Vector3> translate;
		AnimationCurve<Quaternion> rotate;
		AnimationCurve<Vector3> scale;
	};

	struct Animation {
		float duration; // アニメーション全体の尺（単位は秒）
		// NodeAnimationの集合。Node名でひけるようにしておく
		std::map<std::string, NodeAnimation> nodeAnimations;
	};

public:
	// 初期化
	void Initialize(const std::string& directoryPath, const std::string& filename);

	// 描画
	void Draw();

	// アニメーション再生
	void PlayAnimation();

	ModelData GetModelData() const { return modelData_; }

	float GetEnvironmentCoefficient() { return materialData_->environmentCoefficient; }
	void SetEnvironmentCoefficient(float environmentCoefficient) { materialData_->environmentCoefficient = environmentCoefficient; }

	void SetNumInstance(uint32_t numInstance) { numInstance_ = numInstance; }

	Matrix4x4 GetLocalMatrix() { return localMatrix_; }

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

	// 任意の時刻の値を取得する
	Vector3 CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time);
	Quaternion CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time);

	// objファイルのデータ
	ModelData modelData_;

	DirectXBase* dxBase_ = nullptr;

	// アニメーション
	Animation animation_{};
	// アニメーション時間
	float animationTime_ = 0.0f;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_; // 頂点リソース
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData_ = nullptr;
	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_; // マテリアルリソース
	// バッファリソース内のデータを指すポインタ
	Material* materialData_ = nullptr;

	// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;

	uint32_t numInstance_ = 1;

	Matrix4x4 localMatrix_{};
};

