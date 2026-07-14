#pragma once
#include <string>
#include <MathFunction.h>
#include <Transform.h>
#include <wrl.h>
#include <d3d12.h>
#include "DirectXBase.h"
#include "Model.h"
#include "Camera.h"
#include "LightManager.h"
#include "Skybox.h"

class Object3dCommon;

// 3Dオブジェクト
class Object3d {
public: // メンバ関数
	// 初期化
	void Initialize(Object3dCommon* object3dCommon);

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
	void SetModel(const std::string& filePath);

	// setter
	void SetCamera(Camera* camera) { camera_ = camera; }
	
	// setter
	void SetSkybox(Skybox* skybox) { skybox_ = skybox; }

	Model* GetModel() { return model_; }

private:
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

	// 座標変換行列データ作成
	void CreateTransformationMatrixData();

	// カメラデータ作成
	void CreateCameraData();

	// Object3DCommonのポインタ
	Object3dCommon* object3dCommon_ = nullptr;

	// DirectXBase
	DirectXBase* dxBase_ = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource; // 座標返還行列リソース
	// バッファリソース内のデータを指すポインタ
	TransformationMatrix* transformationMatrixData = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource;
	// バッファリソース内のデータを指すポインタ
	CameraForGPU* cameraData = nullptr;

	// Transform
	Transform transform{};

	// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;

	// モデル
	Model* model_ = nullptr;

	// カメラ
	Camera* camera_ = nullptr;

	// ライトマネージャ
	LightManager* lightManager_ = LightManager::GetInstance();

	// スカイボックス
	Skybox* skybox_ = nullptr;
};

