#pragma once
#include <string>
#include <vector>
#include <MathFunction.h>
#include <Transform.h>
#include <wrl.h>
#include <d3d12.h>
#include "DirectXBase.h"
#include "Model.h"
#include "ModelManager.h"
#include "Camera.h"
#include "LightManager.h"

class Object3dCommon;

// 3Dオブジェクト
class Sphere {
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
		float shininess;
	};

	// 座標変換用行列
	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
		Matrix4x4 WorldInverseTranspose;
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

	// カメラ
	struct CameraForGPU {
		Vector3 worldPosition;
	};

	// 初期化
	void Initialize(Object3dCommon* object3dCommon, std::string textureFilePath);

	// 更新
	void Update();

	// 描画
	void Draw();

	// setter
	void SetScale(const Vector3& scale) { transform.scale = scale; }
	void SetRotate(const Vector3& rotate) { transform.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform.translate = translate; }

	// getter
	const Vector3& GetScale() const { return transform.scale; }
	const Vector3& GetRotate() const { return transform.rotate; }
	const Vector3& GetTranslate() const { return transform.translate; }

	// setter
	void SetCamera(Camera* camera) { camera_ = camera; }

private:
	// 頂点データ作成
	void CreateVertexData();

	// マテリアルデータ作成
	void CreateMaterialData();

	// 座標変換行列データ作成
	void CreateTransformationMatrixData();

	// カメラデータ作成
	void CreateCameraData();

	// Object3DCommonのポインタ
	Object3dCommon* object3dCommon_ = nullptr;

	// DirectXBase
	DirectXBase* dxBase_ = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr; // 頂点リソース
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource = nullptr; // インデックスリソース
	// バッファリソース内のデータを指すポインタ
	uint32_t* indexData = nullptr;
	// バッファリソースの使い道を補足するバッファビュー
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = nullptr; // マテリアルリソース
	// バッファリソース内のデータを指すポインタ
	Material* materialData = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource = nullptr; // 座標返還行列リソース
	// バッファリソース内のデータを指すポインタ
	TransformationMatrix* transformationMatrixData = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource = nullptr;
	// バッファリソース内のデータを指すポインタ
	CameraForGPU* cameraData = nullptr;

	// Transform
	Transform transform;

	// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;

	// カメラ
	Camera* camera_ = nullptr;

	uint32_t textureIndex;
	// ファイルパス
	std::string filePath;

	const uint32_t kSubdivision = 16; // 分割数

	LightManager* lightManager_ = LightManager::GetInstance();
};

