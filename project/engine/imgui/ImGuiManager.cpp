#include "ImGuiManager.h"
#include "SrvManager.h"

using namespace Microsoft::WRL;
using namespace std;

unique_ptr<ImGuiManager> ImGuiManager::instance_ = nullptr;

ImGuiManager* ImGuiManager::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = make_unique<ImGuiManager>(ConstructorKey());
	}
	return instance_.get();
}

void ImGuiManager::Initialize() {
#ifdef USE_IMGUI
	// メンバ変数に記録
	dxBase_ = DirectXBase::GetInstance();

	srvHeap_ = SrvManager::GetInstance()->GetDescriptorHeap();

	// ImGuiのコンテキストを生成
	ImGui::CreateContext();
	// ImGuiのスタイルを設定
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(WinApp::GetInstance()->GetHwnd());

	// DirectX12用の初期化情報
	ImGui_ImplDX12_InitInfo initInfo = {};
	// 初期化情報を設定する
	initInfo.Device = dxBase_->GetDevice().Get();
	initInfo.CommandQueue = dxBase_->GetCommandQueue().Get();
	initInfo.NumFramesInFlight = static_cast<int>(dxBase_->GetSwapChainResourcesNum());
	initInfo.RTVFormat = dxBase_->GetRtvDesc().Format;
	initInfo.DSVFormat = dxBase_->GetDsvDesc().Format;
	initInfo.SrvDescriptorHeap = srvHeap_.Get();
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
#endif
}

void ImGuiManager::Begin() {
#ifdef USE_IMGUI
	// ImGuiフレーム開始
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif
}

void ImGuiManager::End() {
#ifdef USE_IMGUI
	// 描画前準備
	ImGui::Render();
#endif
}

void ImGuiManager::Draw() {
#ifdef USE_IMGUI
	ComPtr<ID3D12GraphicsCommandList> commandList = dxBase_->GetCommandList();

	// デスクリプタヒープの配列をセットするコマンド
	ComPtr<ID3D12DescriptorHeap> ppHeaps[] = { srvHeap_ };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps->GetAddressOf());
	// 描画コマンドを発行
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());
#endif
}

void ImGuiManager::Finalize() {
#ifdef USE_IMGUI
	// 後始末
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	instance_.reset();
#endif
}
