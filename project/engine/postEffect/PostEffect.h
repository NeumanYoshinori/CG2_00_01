#pragma once
#include "DirectXBase.h"
#include "Vector3.h"
#include <string>

class PostEffect {
public:
	enum struct PostEffectType {
		FullScreen,
		Grayscale,
		Vignette,
	};

	// シングルトンインスタンスの取得
	static PostEffect* GetInstance();

	// 解放
	void Finalize();

	// 初期化
	void Initialize(DirectXBase* dxBase);

	// 描画
	void Draw();

	// ポストエフェクトを変更
	void SetPostEffect(PostEffectType postEffectType) { postEffectType_ = postEffectType; }

	void SetColor(Vector3 color) { materialData_->color = color; }
	void UseSepia(bool useSepia) { materialData_->useSepia = useSepia; }

	// namespace省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	// コンストラクタに渡すための鍵
	class ConstructorKey {
	private:
		ConstructorKey() = default;
		friend class PostEffect;
	};

	// PassKeyを受け取るコンストラクタ
	explicit PostEffect(ConstructorKey) {}

private:
	// マテリアルデータ
	struct Material {
		Vector3 color;
		bool useSepia;
	};

	// ルートシグネチャの作成
	void CreateRootSignature();

	// グラフィックスパイプラインの生成
	void GenerateGraphicsPipeline(std::wstring psName, PostEffectType postEffectType);

	// マテリアルデータ作成
	void CreateMaterialData();

	// シングルトンインスタンス
	static std::unique_ptr<PostEffect> instance_;

	// DxBase
	DirectXBase* dxBase_ = nullptr;

	// ルートシグネチャ
	ComPtr<ID3D12RootSignature> rootSignature_;

	// グラフィックスパイプライン
	ComPtr<ID3D12PipelineState> graphicsPipelineState_[3];

	// SRVインデックス
	uint32_t srvIndex_ = 0;

	// レンダーテクスチャリソース
	ComPtr<ID3D12Resource> renderTextureResource_;

	// マテリアルリソース
	ComPtr<ID3D12Resource> materialResource_;
	Material* materialData_ = nullptr;

	// ポストエフェクトの種類
	PostEffectType postEffectType_{};

	// PSの名前
	std::wstring psName_;

	~PostEffect() = default;
	PostEffect(const PostEffect&) = delete;
	const PostEffect& operator=(const PostEffect&) = delete;

	// default_delete にアクセスを許可する
	friend struct std::default_delete<PostEffect>;
};


