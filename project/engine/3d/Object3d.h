#pragma once
#include <string>
#include <MathFunction.h>
#include <Transform.h>
#include <wrl.h>
#include "DirectXBase.h"
#include "Model.h"
#include "Camera.h"
#include "LightManager.h"
#include "Primitive.h"

// 3Dオブジェクト
class Object3d {
public: // メンバ関数
	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Draw();

	// デバッグ
	void DebugUpdate();

	// setter
	void SetScale(const Vector3& scale) { transform_.scale = scale; }
	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform_.translate = translate; }

	// getter
	const Vector3& GetScale() const { return transform_.scale; }
	const Vector3& GetRotate() const { return transform_.rotate; }
	const Vector3& GetTranslate() const { return transform_.translate; }

	// setter
	void SetModel(const std::string& filePath);

	// setter
	void SetCamera(Camera* camera) { camera_ = camera; }

	// setter
	void SetEnvironmentMapTexture(const std::string& envMapFilePath) { environmentMapFilePath_ = envMapFilePath; }

	// setter
	void SetPrimitive(Primitive* primitive) { primitive_ = primitive; }

	Model* GetModel() { return model_; }

private:
	// 座標変換用行列
	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
		Matrix4x4 WorldInverseTranspose;
		int32_t flipX;
		int32_t flipY;
	};

	// カメラ
	struct CameraForGPU {
		Vector3 worldPosition;
	};

	struct EnvironmentMap {
		int32_t useEnvironmentMap;
	};

	// 座標変換行列データ作成
	void CreateTransformationMatrixData();

	// カメラデータ作成
	void CreateCameraData();

	// 環境マップデータ作成
	void CreateEnvironmentMapData();

	// DirectXBase
	DirectXBase* dxBase_ = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_; // 座標返還行列リソース
	// バッファリソース内のデータを指すポインタ
	TransformationMatrix* transformationMatrixData_ = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource_;
	// バッファリソース内のデータを指すポインタ
	CameraForGPU* cameraData_ = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> environmentMapResource_;
	// バッファリソース内のデータを指すポインタ
	EnvironmentMap* environmentMapData_ = nullptr;

	// Transform
	Transform transform_{};

	// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;

	// モデル
	Model* model_ = nullptr;

	// カメラ
	Camera* camera_ = nullptr;

	// ライトマネージャ
	LightManager* lightManager_ = LightManager::GetInstance();

	// プリミティブ
	Primitive* primitive_ = nullptr;

	std::string environmentMapFilePath_;
};

