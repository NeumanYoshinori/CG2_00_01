#pragma once
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#endif
#include "DirectXBase.h"

// ImGUIの管理
class ImGuiManager {
public:
	// シングルトンインスタンスの取得
	static ImGuiManager* GetInstance();
	
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// ImGui受付開始
	/// </summary>
	void Begin();

	/// <summary>
	/// ImGui受付終了
	/// </summary>
	void End();

	/// <summary>
	/// 画面への描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 終了
	/// </summary>
	void Finalize();

	// コンストラクタに渡すための鍵
	class ConstructorKey {
	private:
		ConstructorKey() = default;
		friend class ImGuiManager;
	};

	// PassKeyを受け取るコンストラクタ
	explicit ImGuiManager(ConstructorKey) {}

private:
	// シングルトンインスタンス
	static std::unique_ptr<ImGuiManager> instance_;

	// DirectXBase
	DirectXBase* dxBase_ = nullptr;

	// SRVヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_;

	~ImGuiManager() = default;
	ImGuiManager(ImGuiManager&) = delete;
	ImGuiManager& operator=(ImGuiManager&) = delete;

	// default_delete にアクセスを許可する
	friend struct std::default_delete<ImGuiManager>;
};

