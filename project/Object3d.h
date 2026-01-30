#pragma once
#include <string>
#include <vector>
#include <MathFunction.h>
#include <Transform.h>
#include <wrl.h>
#include <d3d12.h>

class Object3dCommon;
class DirectXBase;

// 3Dオブジェクト
class Object3d {
public: // メンバ関数
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

	// 座標変換用行列
	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
	};

	// 平行光源
	struct DirectionalLight {
		Vector4 color;
		Vector3 direction;
		float intensity;
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
	void Initialize(Object3dCommon* object3dCommon);

	// 更新
	void Update();

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

	// 座標変換行列データ作成
	void CreateTransformationMatrixData();

	// 平行光源データ作成
	void CreateDirectionalLight();

	Object3dCommon* object3dCommon_ = nullptr;

	// objファイルのデータ
	ModelData modelData;

	DirectXBase* dxBase_ = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr; // 頂点リソース
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = nullptr; // マテリアルリソース
	// バッファリソース内のデータを指すポインタ
	Material* materialData = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource = nullptr; // 座標返還行列リソース
	// バッファリソース内のデータを指すポインタ
	TransformationMatrix* transformationMatrixData = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = nullptr;
	// バッファリソース内のデータを指すポインタ
	DirectionalLight* directionalLightData = nullptr;

	Transform transform;
	Transform cameraTransform;

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
};

