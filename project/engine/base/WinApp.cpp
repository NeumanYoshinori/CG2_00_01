#include "WinApp.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND HwND, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

#pragma comment(lib, "winmm.lib")

using namespace std;

unique_ptr<WinApp> WinApp::instance_ = nullptr;

WinApp* WinApp::GetInstance() {
	if (instance_ == nullptr) {
		// PassKeyを渡してインスタンス生成
		instance_ = make_unique<WinApp>(ConstructorKey());
	}
	return instance_.get();
}

// ウィンドウプロシージャ
LRESULT CALLBACK WinApp::WindowProc(HWND hwnd, UINT msg,
	WPARAM wparam, LPARAM lparam) {
#ifdef USE_IMGUI
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}
#endif
	// メッセージに応じてゲーム固有の処理を行う
	switch (msg) {
	case WM_DESTROY: // ウィンドウが破棄された
		// OSに対して、アプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}

	// 標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void WinApp::Initialize() {
	HRESULT hr = CoInitializeEx(0, COINITBASE_MULTITHREADED);

	// ウィンドウプロシージャ
	wc_.lpfnWndProc = WindowProc;
	// ウィンドウクラス名
	wc_.lpszClassName = L"CG2WindowClass";
	// インスタンスハンドル
	wc_.hInstance = GetModuleHandle(nullptr);
	// カーソル
	wc_.hCursor = LoadCursor(nullptr, IDC_ARROW);

	// ウィンドウクラスを登録する
	RegisterClass(&wc_);

	// ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0, 0, kClientWidth, kClientHeight };

	// クライアント領域をもとに実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	hwnd_ = CreateWindow(
		wc_.lpszClassName,		// 利用するクラス名
		L"CG2",					// タイトルバーバーの文字
		WS_OVERLAPPEDWINDOW,	// よく見るウィンドウスタイル
		CW_USEDEFAULT,			// 表示X座標
		CW_USEDEFAULT,			// 表示Y座標
		wrc.right - wrc.left,	// ウィンドウ横幅
		wrc.bottom - wrc.top,	// ウィンドウ縦幅
		nullptr,				// 親ウィンドウハンドル
		nullptr,				// メニューハンドル
		wc_.hInstance,			// インスタントハンドル
		nullptr);				// オプション

	// ウィンドウを表示する
	ShowWindow(hwnd_, SW_SHOW);

	// システムタイマーの分解能を上げる
	timeBeginPeriod(1);
}

bool WinApp::ProcessMessage() {
	MSG msg{};

	// Windowにメッセージが来てたら最優先で処理させる
	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// ウィンドウの×ボタンが押されるまでループ
	if (msg.message == WM_QUIT) {
		return true;
	}

	return false;
}

void WinApp::Finalize() {
	CloseWindow(hwnd_);
	CoUninitialize();
}