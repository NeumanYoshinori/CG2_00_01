#pragma once
#include "MathFunction.h"
#include "DirectXBase.h"

class Primitive {
public:
	// 初期化
	void Initialize(std::string textureFilePath, uint32_t numInstance);

	// 描画
	virtual void Draw() = 0;

	virtual ~Primitive() = default;

	float GetEnvironmentCoefficient() { return materialData_->environmentCoefficient; }
	void SetEnvironmentCoefficient(float environmentCoefficient) { materialData_->environmentCoefficient = environmentCoefficient; }

	void SetNumInstance(uint32_t numInstance) { numInstance_ = numInstance; }

	// namespace省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

protected:
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
		float alphaReference;
		float padding2;
	};

	virtual void CreateVertexData() = 0;

	void CreateMaterialData();

	// DirectXBase
	DirectXBase* dxBase_ = nullptr;

	// 頂点リソース
	ComPtr<ID3D12Resource> vertexResource_;

	VertexData* vertexData_ = nullptr;
	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	ComPtr<ID3D12Resource> indexResource_;
	uint32_t* indexData_ = nullptr;
	// インデックスバッファビューを作成する
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

	// マテリアルリソース
	ComPtr<ID3D12Resource> materialResource_;
	Material* materialData_ = nullptr;

	// コマンドリスト
	ComPtr<ID3D12GraphicsCommandList> commandList_;

	// 頂点数
	const uint32_t kNumVertex_ = 4;
	// インデックス数
	const uint32_t kNumIndex_ = 6;

	// ファイルパス
	std::string textureFilePath_;

	uint32_t numInstance_ = 0;
};
