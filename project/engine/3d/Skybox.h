#pragma once
#include <string>
#include <MathFunction.h>
#include <Transform.h>
#include <wrl.h>
#include <d3d12.h>
#include "DirectXBase.h"
#include "Camera.h"

// スカイボックス
class Skybox {
public:
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

	// ファイルパス取得
	std::string GetFilePath() { return filePath_; }

	// namespace省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

private:
	// 頂点データ
	struct VertexData {
		Vector4 position;
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

	// マテリアルデータ
	struct MaterialData {
		std::string textureFilePath;
		uint32_t textureIndex = 0;
	};

	// 頂点データ作成
	void CreateVertexData();

	// マテリアルデータ作成
	void CreateMaterialData();

	// 座標変換行列データ作成
	void CreateTransformationMatrixData();

	// DirectXBase
	DirectXBase* dxBase_ = nullptr;

	// バッファリソース
	ComPtr<ID3D12Resource> vertexResource_ = nullptr; // 頂点リソース
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData_ = nullptr;
	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	const uint32_t kNumVertex_ = 24; // 頂点数

	// バッファリソース
	ComPtr<ID3D12Resource> indexResource_ = nullptr; // インデックスリソース
	// バッファリソース内のデータを指すポインタ
	uint32_t* indexData_ = nullptr;
	// バッファリソースの使い道を補足するバッファビュー
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

	const uint32_t kNumIndex_ = 36; // インデックス数

	const uint32_t kNumFace_ = 6; // 面の数

	// バッファリソース
	ComPtr<ID3D12Resource> materialResource_ = nullptr; // マテリアルリソース
	// バッファリソース内のデータを指すポインタ
	Material* materialData_ = nullptr;

	// バッファリソース
	ComPtr<ID3D12Resource> transformationMatrixResource_; // 座標返還行列リソース
	// バッファリソース内のデータを指すポインタ
	TransformationMatrix* transformationMatrixData_ = nullptr;

	// Transform
	Transform transform_{};

	// コマンドリスト
	ComPtr<ID3D12GraphicsCommandList> commandList_;

	// カメラ
	Camera* camera_ = nullptr;

	// ファイルパス
	std::string filePath_;
};

