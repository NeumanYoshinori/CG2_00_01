#pragma once
#include "MathFunction.h"
#include <string>
#include <vector>
#include "DirectXBase.h"
#include <assimp/scene.h>

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

public:
	// 初期化
	void Initialize(const std::string& directorypath, const std::string& filename);

	// 描画
	void Draw();

	// .objファイルの読み取り
	static ModelData LoadModelFile(const std::string& directoryPath, const std::string& filename);

	static Node ReadNode(aiNode* node);

	ModelData GetModelData() const { return modelData_; }

	float GetEnvironmentCoefficient() { return materialData_->environmentCoefficient; }
	void SetEnvironmentCoefficient(float environmentCoefficient) { materialData_->environmentCoefficient = environmentCoefficient; }

	void SetNumInstance(uint32_t numInstance) { numInstance_ = numInstance; }

private:
	// 頂点データ作成
	void CreateVertexData();

	// マテリアルデータ作成
	void CreateMaterialData();

	// objファイルのデータ
	ModelData modelData_;

	DirectXBase* dxBase_ = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_; // 頂点リソース
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData_ = nullptr;
	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	// バッファリソース内のデータを指すポインタ
	uint32_t* indexData_ = nullptr;
	// バッファリソースの使い道を補足するバッファビュー
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_; // マテリアルリソース
	// バッファリソース内のデータを指すポインタ
	Material* materialData_ = nullptr;

	// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;

	uint32_t numInstance_ = 0;
};

