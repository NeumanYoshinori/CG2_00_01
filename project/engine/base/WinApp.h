#pragma once
#include <Windows.h>
#include <cstdint>
#include <memory>

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
	HWND GetHwnd() const { return hwnd_; }
	HINSTANCE GetHInstance() const { return wc_.hInstance; }

	// メッセージの処理
	bool ProcessMessage();

	// 終了
	void Finalize();

	// コンストラクタに渡すための鍵
	class ConstructorKey {
	private:
		ConstructorKey() = default;
		friend class WinApp;
	};

	// PassKeyを受け取るコンストラクタ
	explicit WinApp(ConstructorKey) {}

private:
	// インスタンス
	static std::unique_ptr<WinApp> instance_;

	// ウィンドウハンドル
	HWND hwnd_ = nullptr;

	// ウィンドウクラスの設定
	WNDCLASS wc_{};

	~WinApp() = default;
	WinApp(const WinApp&) = delete;
	const WinApp& operator=(const WinApp&) = delete;

	// default_delete にアクセスを許可する
	friend struct std::default_delete<WinApp>;
};

