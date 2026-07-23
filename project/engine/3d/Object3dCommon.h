#pragma once
#include <wrl.h>
#include <d3d12.h>
#include "DirectXBase.h"
#include "Camera.h"

// 3Dオブジェクト共通部
class Object3dCommon {
public: // メンバ関数
	// ブレンドモード
	enum BlendMode {
		//!< ブレンドなし
		kBlendModeNone,
		//!< 通常αブレンド。Src * SrcA + Dest * (1 - SrcA)
		kBlendModeNormal,
		//!< 加算。Src * SrcA + Dest * 1
		kBlendModeAdd,
		//!< 減算。Dest * 1 - Src * SrcA
		kBlendModeSubtract,
		//!< 乗算。Src * 0 + Dest * Src
		kBlendModeMultiply,
		//!< スクリーン。Src * (1 - Dest) + Dest * 1
		kBlendModeScreen,
		// 利用してはいけない
		kCountOfBlendMode,
	};

	// シングルトンインスタンスの取得
	static Object3dCommon* GetInstance();

	// 終了
	void Finalize();

	// 初期化
	void Initialize();

	// 共通描画設定
	void DrawSetting();

	// setter
	void SetDefaultCamera(Camera* camera) { defaultCamera_ = camera; }
	// getter
	Camera* GetDefaultCamera() const { return defaultCamera_; }

	// コンストラクタに渡すための鍵
	class ConstructorKey {
	private:
		ConstructorKey() = default;
		friend class Object3dCommon;
	};

	// PassKeyを受け取るコンストラクタ
	explicit Object3dCommon(ConstructorKey) {}

private:
	// ルートシグネチャの作成
	void CreateRootSignature();
	// グラフィックスパイプラインの生成
	void GenerateGraphicsPipeLine();

	// インスタンス
	static std::unique_ptr<Object3dCommon> instance_;

	// DirectXBase
	DirectXBase* dxBase_ = nullptr;

	// コマンドリストを生成する
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;

	// ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

	// グラフィックスパイプラインステート
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;

	// デフォルトカメラ
	Camera* defaultCamera_ = nullptr;

	~Object3dCommon() = default;
	Object3dCommon(Object3dCommon&) = delete;
	Object3dCommon& operator=(Object3dCommon&) = delete;

	// default_delete にアクセスを許可する
	friend struct std::default_delete<Object3dCommon>;
};

