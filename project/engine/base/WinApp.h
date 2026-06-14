#pragma once
#include <Windows.h>
#include <cstdint>

// WindowsAPI
class WinApp {
public:
	// シングルトンインスタンスの取得
	static WinApp* GetInstance();

	// ウィンドウプロシージャ
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	// 初期化
	void Initialize();

	// クライアント領域のサイズ
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;

	// getter
	HWND GetHwnd() const { return hwnd; }
	HINSTANCE GetHInstance() const { return wc.hInstance; }

	// メッセージの処理
	bool ProcessMessage();

	// 終了
	void Finalize();

private:
	// インスタンス
	static WinApp* instance;

	// ウィンドウハンドル
	HWND hwnd = nullptr;

	// ウィンドウクラスの設定
	WNDCLASS wc{};

	WinApp() = default;
	~WinApp() = default;
	WinApp(const WinApp&) = delete;
	const WinApp& operator=(const WinApp&) = delete;
};

