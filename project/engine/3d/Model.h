#pragma once
#include "MathFunction.h"
#include <string>
#include <vector>
#include "DirectXBase.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class ModelCommon;

// 3Dモデル
class Model {
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
		float shininess;
	};

	// マテリアルデータ
	struct MaterialData {
		std::string textureFilePath;
		uint32_t textureIndex = 0;
	};

	// ノード
	struct Node {
		Matrix4x4 localMatrix;
		std::string name;
		std::vector<Node> children;
	};

	// モデルデータ
	struct ModelData {
		std::vector<VertexData> vertices;
		MaterialData material;
		Node rootNode;
	};

	// 初期化
	void Initialize(ModelCommon* modelCommon, const std::string& directorypath, const std::string& filename);

	// 描画
	void Draw();

	// .objファイルの読み取り
	static ModelData LoadModelFile(const std::string& directoryPath, const std::string& filename);

	static Node ReadNode(aiNode* node);

	ModelData GetModelData() const { return modelData; }

private:
	// 頂点データ作成
	void CreateVertexData();

	// マテリアルデータ作成
	void CreateMaterialData();

	// ModelCommonのポインタ
	ModelCommon* modelCommon_ = nullptr;

	// objファイルのデータ
	ModelData modelData;

	DirectXBase* dxBase_ = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr; // 頂点リソース
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};

	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource = nullptr;
	uint32_t* indexData = nullptr;

	D3D12_INDEX_BUFFER_VIEW indexBufferView{};

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = nullptr; // マテリアルリソース
	// バッファリソース内のデータを指すポインタ
	Material* materialData = nullptr;

	// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
};

