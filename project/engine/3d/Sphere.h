#pragma once
#include <string>
#include <MathFunction.h>
#include <Transform.h>
#include <wrl.h>
#include <d3d12.h>
#include "DirectXBase.h"
#include "Camera.h"
#include "LightManager.h"
#include "Skybox.h"

class Object3dCommon;

// 球
class Sphere {
public: // メンバ関数
	// 初期化
	void Initialize(std::string textureFilePath);

	// 更新
	void Update();

	// 描画
	void Draw();

	// setter
	void SetScale(const Vector3& scale) { transform_.scale = scale; }
	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform_.translate = translate; }

	// getter
	const Vector3& GetScale() const { return transform_.scale; }
	const Vector3& GetRotate() const { return transform_.rotate; }
	const Vector3& GetTranslate() const { return transform_.translate; }

	// setter
	void SetCamera(Camera* camera) { camera_ = camera; }

	// setter
	void SetSkybox(Skybox* skybox) { skybox_ = skybox; }

	float GetEnvironmentCoefficient() { return materialData_->environmentCoefficient; }
	void SetEnvironmentCoefficient(float environmentCoefficient) { materialData_->environmentCoefficient = environmentCoefficient; }

	// namespace省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

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
		float padding2[2];
	};

	// 座標変換用行列
	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
		Matrix4x4 WorldInverseTranspose;
	};

	// カメラ
	struct CameraForGPU {
		Vector3 worldPosition;
	};

	// 頂点データ作成
	void CreateVertexData();

	// マテリアルデータ作成
	void CreateMaterialData();

	// 座標変換行列データ作成
	void CreateTransformationMatrixData();

	// カメラデータ作成
	void CreateCameraData();

	// DirectXBase
	DirectXBase* dxBase_ = nullptr;

	// バッファリソース
	ComPtr<ID3D12Resource> vertexResource_; // 頂点リソース
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData_ = nullptr;
	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	// バッファリソース
	ComPtr<ID3D12Resource> indexResource_; // インデックスリソース
	// バッファリソース内のデータを指すポインタ
	uint32_t* indexData_ = nullptr;
	// バッファリソースの使い道を補足するバッファビュー
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

	// バッファリソース
	ComPtr<ID3D12Resource> materialResource_; // マテリアルリソース
	// バッファリソース内のデータを指すポインタ
	Material* materialData_ = nullptr;

	// バッファリソース
	ComPtr<ID3D12Resource> transformationMatrixResource_; // 座標返還行列リソース
	// バッファリソース内のデータを指すポインタ
	TransformationMatrix* transformationMatrixData_ = nullptr;

	// バッファリソース
	ComPtr<ID3D12Resource> cameraResource_;
	// バッファリソース内のデータを指すポインタ
	CameraForGPU* cameraData_ = nullptr;

	// Transform
	Transform transform_{};

	// コマンドリスト
	ComPtr<ID3D12GraphicsCommandList> commandList_;

	// カメラ
	Camera* camera_ = nullptr;

	// マテリアル
	std::string textureFilePath_;

	const uint32_t kSubdivision_ = 32; // 分割数

	LightManager* lightManager_ = LightManager::GetInstance();

	// スカイボックス
	Skybox* skybox_ = nullptr;
};

