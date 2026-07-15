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
	// 初期化
	void Initialize(SpriteCommon* spriteCommon, std::string textureFilePath);

	// 更新
	void Update();

	// 描画
	void Draw();

	// 座標
	// getter
	const Vector2& GetPosition() const { return position_; }
	// setter
	void SetPosition(const Vector2& position) { position_ = position; }

	// 回転
	float GetRotation() const { return rotation_; }
	void SetRotation(float rotation) { rotation_ = rotation; }

	// 色
	const Vector4& GetColor() const { return materialData_->color; }
	void SetColor(const Vector4& color) { materialData_->color = color; }

	// サイズ
	const Vector2& GetSize() const { return size_; }
	void SetSize(const Vector2& size) { size_ = size; }

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
		textureLeftTop_.x = leftTopX;
		textureLeftTop_.y = leftTopY;
	}

	// テクスチャ切り出しサイズのsetter
	void SetTextureSize(float sizeX, float sizeY) {
		textureSize_.x = sizeX;
		textureSize_.y = sizeY;
	}

	// テクスチャ左上座標のgetter
	const Vector2& GetTextureLeftTop() const { return textureLeftTop_; }
	// テクスチャ切り出しサイズのgetter
	const Vector2& GetTextureSize() const { return textureSize_; }

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
		Matrix4x4 uvTransform;
	};

	// 座標変換用行列
	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
	};

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
	ComPtr<ID3D12Resource> vertexResource_; // 頂点リソース
	ComPtr<ID3D12Resource> indexResource_; // インデックスリソース
	ComPtr<ID3D12Resource> materialResource_; // マテリアルリソース
	ComPtr<ID3D12Resource> transformationMatrixResource_; // 座標変換行列リソース

	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData_ = nullptr;
	uint32_t* indexData_ = nullptr;
	Material* materialData_ = nullptr;
	TransformationMatrix* transformationMatrixData_ = nullptr;

	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

	// DirectXBase
	DirectXBase* dxBase_ = nullptr;

	// コマンドリスト
	ComPtr<ID3D12GraphicsCommandList> commandList_;

	// テクスチャリソース
	ComPtr<ID3D12Resource> textureResource_;
	ComPtr<ID3D12Resource> intermediateResource_;

	// 座標
	Vector2 position_ = { 0.0f, 0.0f };

	// 回転
	float rotation_ = 0.0f;

	// サイズ
	Vector2 size_ = { 640.0f, 360.0f };

	// アンカーポイント
	Vector2 anchorPoint_ = { 0.0f, 0.0f };

	// 左右フリップ
	bool isFlipX_ = false;
	// 上下フリップ
	bool isFlipY_ = false;

	// テクスチャ左上座標
	Vector2 textureLeftTop_ = { 0.0f, 0.0f };
	// テクスチャ切り出しサイズ
	Vector2 textureSize_ = { 100.0f, 100.0f };

	// transform
	Transform transform_{};

	// ファイルパス
	std::string filePath_;
};

