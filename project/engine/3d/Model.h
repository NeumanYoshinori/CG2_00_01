#pragma once
#include "MathFunction.h"
#include <string>
#include <vector>
#include "DirectXBase.h"

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
	};

	// マテリアルデータ
	struct MaterialData {
		std::string textureFilePath;
		uint32_t textureIndex = 0;
	};

	// モデルデータ
	struct ModelData {
		std::vector<VertexData> verticles;
		MaterialData material;
	};

	// 初期化
	void Initialize(ModelCommon* modelCommon, const std::string& directorypath, const std::string& filename);

	// 描画
	void Draw();

	// .mtlファイルの読み取り
	static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
	// .objファイルの読み取り
	static ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

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

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = nullptr; // マテリアルリソース
	// バッファリソース内のデータを指すポインタ
	Material* materialData = nullptr;

	// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
};

