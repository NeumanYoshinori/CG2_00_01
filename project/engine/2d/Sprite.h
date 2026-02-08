#pragma once
#include "MathFunction.h"
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>
#include <string>
#include "Transform.h"
#include "DirectXBase.h"

class SpriteCommon;

// スプライト
class Sprite {
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

	// 初期化
	void Initialize(SpriteCommon* spriteCommon, std::string textureFilePath);

	// 更新
	void Update();

	// 描画
	void Draw();

	// 座標
	// getter
	const Vector2& GetPosition() const { return position; }
	// setter
	void SetPosition(const Vector2& position) { this->position = position; }

	// 回転
	float GetRotation() const { return rotation; }
	void SetRotation(float rotation) { this->rotation = rotation; }

	// 色
	const Vector4& GetColor() const { return materialData->color; }
	void SetColor(const Vector4& color) { materialData->color = color; }

	// サイズ
	const Vector2& GetSize() const { return size; }
	void SetSize(const Vector2& size) { this->size = size; }

	// アンカーポイント
	const Vector2& GetAnchorPoint() const { return anchorPoint_; }
	// setter
	void SetAnchorPoint(const Vector2& anchorPoint) { anchorPoint_ = anchorPoint; }

	// フリップのsetter
	void SetFlipX(bool isFlipX) { isFlipX_ = isFlipX; }
	void SetFlipY(bool isFlipY) { isFlipY_ = isFlipY; }

	// フリップのgetter
	bool IsFlipX() const { return isFlipX_; }
	bool IsFlipY() const { return isFlipY_; }

	// テクスチャ左上座標のsetter
	void SetTextureLeftTop(float leftTopX, float leftTopY) {
		textureLeftTop.x = leftTopX;
		textureLeftTop.y = leftTopY;
	}

	// テクスチャ切り出しサイズのsetter
	void SetTextureSize(float sizeX, float sizeY) {
		textureSize.x = sizeX;
		textureSize.y = sizeY;
	}

	// テクスチャ左上座標のgetter
	const Vector2& GetTextureLeftTop() const { return textureLeftTop; }
	// テクスチャ切り出しサイズのgetter
	const Vector2& GetTextureSize() const { return textureSize; }

private:
	// 頂点データ作成
	void CreateVertexData();

	// マテリアルデータ作成
	void CreateMaterialData();

	// 座標変換行列データ作成
	void CreateTransformationMatrixData();

	// テクスチャサイズをイメージに合わせる
	void AdjustTextureSize();

	// SpriteCommonのポインタ
	SpriteCommon* spriteCommon_ = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr; // 頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource = nullptr; // インデックスリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = nullptr; // マテリアルリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource = nullptr; // 座標変換行列リソース

	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	uint32_t* indexData = nullptr;
	Material* materialData = nullptr;
	TransformationMatrix* transformationMatrixData = nullptr;

	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	// DirectXBase
	DirectXBase* dxBase_ = nullptr;

	// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;

	// テクスチャリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = nullptr;

	// 座標
	Vector2 position = { 0.0f, 0.0f };

	// 回転
	float rotation = 0.0f;

	// サイズ
	Vector2 size = { 640.0f, 360.0f };

	// テクスチャ番号
	uint32_t textureIndex = 0;

	// アンカーポイント
	Vector2 anchorPoint_ = { 0.0f, 0.0f };

	// 左右フリップ
	bool isFlipX_ = false;
	// 上下フリップ
	bool isFlipY_ = false;

	// テクスチャ左上座標
	Vector2 textureLeftTop = { 0.0f, 0.0f };
	// テクスチャ切り出しサイズ
	Vector2 textureSize = { 100.0f, 100.0f };

	// transform
	Transform transform;

	std::string filePath;
};

