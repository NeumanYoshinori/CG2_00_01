#pragma once
#include "DirectXBase.h"
#include "MathFunction.h"
#include "Transform.h"

class Line {
public:
	// 初期化
	void Initialize(Vector2 start, Vector2 end, Vector4 color);

	void Update();

	void Draw();

	// 始点を設定
	void SetStart(Vector2 start) { start_ = start; }
	// 終点を設定
	void SetEnd(Vector2 end) { end_ = end; }

	// 始点と終点を設定
	void SetStartAndEnd(Vector2 start, Vector2 end) { start_ = start; end_ = end; }

private:
	// 頂点データ
	struct VertexData {
		Vector4 position;
	};

	// マテリアルデータ
	struct Material {
		Vector4 color;
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

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_; // 頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_; // マテリアルリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_; // 座標変換行列リソース

	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData_ = nullptr;
	Material* materialData_ = nullptr;
	TransformationMatrix* transformationMatrixData_ = nullptr;

	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	DirectXBase* dxBase_ = nullptr;

	// コマンドリストを生成する
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;

	// 始点
	Vector2 start_{};
	// 終点
	Vector2 end_{};

	// transform
	Transform transform_{};

	// 色
	Vector4 color_{};
};

