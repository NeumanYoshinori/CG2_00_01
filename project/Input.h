#pragma once
#include <Windows.h>
#include <wrl.h>
#define DIRECTINPUT_VERSION 0x0800 // DirectInputのバージョン指定
#include <dinput.h>

class Input {
public:
	// 初期化
	void Initialize(HINSTANCE hInstance, HWND hwnd);

	// 更新
	void Update();

	// namespace省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	// キーを押したかをチェック
	bool PushKey(BYTE keyNumber);
	// キーが押されていないかチェック
	bool NotPushKey(BYTE keyNumber);
	// トリガー処理
	bool TriggerKey(BYTE keyNumber);
	// 離した瞬間の処理
	bool ReleaseKey(BYTE keyNumber);

private:
	// キーボード
	ComPtr<IDirectInputDevice8> keyboard;

	// 全キーの状態
	BYTE key[256] = {};
	BYTE keyPre[256] = {};

	// DirectInputのインスタンス
	ComPtr<IDirectInput8> directInput;
};