#pragma once
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "WinApp.h"
#include "DirectXBase.h"
#include "SrvManager.h"

// ImGUIの管理
class ImGuiManager {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(WinApp* winApp, DirectXBase* directXBase, SrvManager* srvManager);
};

