#pragma once
#include <Windows.h>
#include <Vector2.h>
#include <Vector3.h>
#include <Vector4.h>
#include <Matrix4x4.h>
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>

class SpriteCommon;
class DirectXBase;

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

// スプライト
class Sprite {
public:
	// 初期化
	void Initialize(SpriteCommon* spriteCommon);

	// 更新
	void Update();

	// 描画
	void Draw();

	// getter
	const Vector2& GetPosition() const { return position; }
	// setter
	void SetPosition(const Vector2& position) { this->position = position; }

	float GetRotation() const { return rotation; }
	void SetRotation(float rotation) { this->rotation = rotation; }

	const Vector4& GetColor() const { return materialData->color; }
	void SetColor(const Vector4& color) { materialData->color = color; }

	const Vector2& GetSize() const { return size; }
	void SetSize(const Vector2& size) { this->size = size; }

private:
	// 頂点データ作成
	void CreateVertexData();

	// マテリアルデータ作成
	void CreateMaterialData();

	// 座標変換行列データ作成
	void CreateTransformationMatrixData();

	SpriteCommon* spriteCommon = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr; // 頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource = nullptr; // インデックスリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = nullptr; // マテリアルリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource = nullptr; // 座標変換リソース

	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	uint32_t* indexData = nullptr;
	Material* materialData = nullptr;
	TransformationMatrix* transformationMatrixData = nullptr;

	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	DirectXBase* dxBase_ = nullptr;

	// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = nullptr;

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU;

	// 座標
	Vector2 position = { 0.0f, 0.0f };

	// 回転
	float rotation = 0.0f;

	// サイズ
	Vector2 size = { 640.0f, 360.0f };
};

