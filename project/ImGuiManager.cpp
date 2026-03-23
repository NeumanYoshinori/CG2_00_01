#include "ImGuiManager.h"

void ImGuiManager::Initialize(WinApp* winApp, DirectXBase* directXBase, SrvManager* srvManager) {
	// ImGuiのコンテキストを生成
	ImGui::CreateContext();
	// ImGuiのスタイルを設定
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(winApp->GetHwnd());

	// DirectX12用の初期化情報
	ImGui_ImplDX12_InitInfo initInfo = {};
	// 初期化情報を設定する
	initInfo.Device = directXBase->GetDevice().Get();
	initInfo.CommandQueue = directXBase->GetCommandQueue().Get();
	initInfo.NumFramesInFlight = static_cast<int>(directXBase->GetSwapChainResourcesNum());
	initInfo.RTVFormat = directXBase->GetRtvDesc().Format;
	initInfo.DSVFormat = directXBase->GetDsvDesc().Format;
	initInfo.SrvDescriptorHeap = srvManager->GetDescriptorHeap().Get();
	// SRV解放用関数の設定
	initInfo.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle,
	D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) {
		SrvManager* srvManager = SrvManager::GetInstance();
		uint32_t index = srvManager->Allocate();
		*out_cpu_handle = srvManager->GetCPUDescriptorHandle(index);
		*out_gpu_handle = srvManager->GetGPUDescriptorHandle(index);
	};
	
	// SRV解放用関数の設定
	initInfo.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle,
	D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) {
		// SrvManagerに解放機能を作っていないため、ここでは何もしない
	};

	// DirectX12用の初期化を行う
	ImGui_ImplDX12_Init(&initInfo);
}
