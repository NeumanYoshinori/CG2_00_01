#pragma once
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#endif
#include "DirectXBase.h"
#include "SrvManager.h"
#include "WinApp.h"

// ImGUIの管理
class ImGuiManager {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(WinApp* winApp, DirectXBase* dxBase);

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

private:
	// DirectXBase
	DirectXBase* dxBase_;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_ = nullptr;
};

