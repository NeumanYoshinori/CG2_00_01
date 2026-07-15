#pragma once
#include <Windows.h>
#include <wrl.h>
#define DIRECTINPUT_VERSION 0x0800 // DirectInputのバージョン指定
#include <dinput.h>
#include <memory>

class Input {	
public:
	// シングルトンインスタンスの取得
	static Input* GetInstance();

	// 初期化
	void Initialize();

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

	// コンストラクタに渡すための鍵
	class ConstructorKey {
	private:
		ConstructorKey() = default;
		friend class Input;
	};

	// PassKeyを受け取るコンストラクタ
	explicit Input(ConstructorKey) {}

private:
	// インスタンス
	static std::unique_ptr<Input> instance_;

	// キーボード
	ComPtr<IDirectInputDevice8> keyboard;

	// 全キーの状態
	BYTE key[256] = {};
	BYTE keyPre[256] = {};

	// DirectInputのインスタンス
	ComPtr<IDirectInput8> directInput;

	~Input() = default;
	Input(Input&) = delete;
	Input& operator=(Input&) = delete;

	// default_delete にアクセスを許可する
	friend struct std::default_delete<Input>;
};