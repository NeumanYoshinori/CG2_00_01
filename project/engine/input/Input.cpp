#include "Input.h"
#include <cassert>
#include "WinApp.h"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

using namespace std;

unique_ptr<Input> Input::instance_ = nullptr;

Input* Input::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = make_unique<Input>(ConstructorKey());
	}
	return instance_.get();
}

void Input::Finalize() {
	instance_.reset();
}

void Input::Initialize() {
	HRESULT result;
	result = DirectInput8Create(WinApp::GetInstance()->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput_, nullptr);
	assert(SUCCEEDED(result));

	// キーボードデバイスの生成
	result = directInput_->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
	assert(SUCCEEDED(result));

	// 入力データ形式のセット
	result = keyboard_->SetDataFormat(&c_dfDIKeyboard); // 標準形式
	assert(SUCCEEDED(result));

	// 排他制御レベルのセット
	result = keyboard_->SetCooperativeLevel(WinApp::GetInstance()->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));

	// マウスデバイスの生成
	result = directInput_->CreateDevice(GUID_SysMouse, &devMouse_, NULL);
	assert(SUCCEEDED(result));

	// 入力データ形式のセット
	result = devMouse_->SetDataFormat(&c_dfDIMouse2); // 標準形式
	assert(SUCCEEDED(result));

	// 排他制御レベルのセット
	result = devMouse_->SetCooperativeLevel(WinApp::GetInstance()->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	assert(SUCCEEDED(result));
}

void Input::Update() {
	// 前回のキー入力を保持
	memcpy(keyPre_, key_, sizeof(key_));

	// キーボード情報の取得開始
	keyboard_->Acquire();

	keyboard_->GetDeviceState(sizeof(key_), key_);

	// 前回のマウス入力を保持
	memcpy(&mousePre_, &mouse_, sizeof(mouse_));

	// マウス情報の取得開始
	devMouse_->Acquire();

	devMouse_->GetDeviceState(sizeof(mouse_), &mouse_);
}

bool Input::PushKey(BYTE keyNumber) {
	// 指定キーを押していればtrueを返す
	if (key_[keyNumber]) {
		return true;
	}
	// そうでなければfalseを返す
	return false;
}

bool Input::NotPushKey(BYTE keyNumber) {
	// 指定キーを押していればtrueを返す
	if (!key_[keyNumber]) {
		return true;
	}
	// そうでなければfalseを返す
	return false;

}

bool Input::TriggerKey(BYTE keyNumber) {
	// 指定キーを押した瞬間trueを返す
	if (!keyPre_[keyNumber] && key_[keyNumber]) {
		return true;
	}
	// そうでなければfalseを返す
	return false;
}

bool Input::ReleaseKey(BYTE keyNumber) {
	// 指定キーを離した瞬間trueを返す
	if (keyPre_[keyNumber] && !key_[keyNumber]) {
		return true;
	}
	// そうでなければfalseを返す
	return false;
}

bool Input::IsPressMouse(int buttonNumber) {
	// 指定ボタンを押していればtrueを返す
	if (mouse_.rgbButtons[buttonNumber]) {
		return true;
	}
	// そうでなければfalseを返す
	return false;
}

bool Input::IsTriggerMouse(int buttonNumber) {
	// 指定ボタンを押した瞬間trueを返す
	if (!mousePre_.rgbButtons[buttonNumber] && mouse_.rgbButtons[buttonNumber]) {
		return true;
	}
	// そうでなければfalseを返す
	return false;
}