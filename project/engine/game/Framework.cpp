#include "Framework.h"
#include "CrashHandler.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include "Object3dCommon.h"
#include "ModelManager.h"
#include "LightManager.h"

void Framework::Initialize() {
	// 誰も補足しなかった場合に(Unhandled)、補足する関数を登録
	SetUnhandledExceptionFilter(CrashHandler::ExportDump);

	// ログの生成
	Logger::GenerateLog();

	// WindowsAPIの初期化
	winApp_ = WinApp::GetInstance();
	winApp_->Initialize();

	// DirectXの初期化
	dxBase_ = DirectXBase::GetInstance();
	dxBase_->Initialize(winApp_);

	// 入力の初期化
	Input::GetInstance()->Initialize(winApp_);

	// SRVマネージャの初期化
	srvManager_ = SrvManager::GetInstance();
	srvManager_->Initialize(dxBase_);

	// テクスチャマネージャの初期化
	TextureManager::GetInstance()->Initialize(dxBase_, srvManager_);

	// スプライト共通部の初期化
	SpriteCommon::GetInstance()->Initialize(dxBase_);

	// ライトマネージャの初期化
	LightManager::GetInstance()->Initialize(dxBase_);

	// モデルマネージャー
	// 3Dモデルマネージャの初期化
	ModelManager::GetInstance()->Initialize(dxBase_);

	// 3Dオブジェクト共通部の初期化
	Object3dCommon::GetInstance()->Initialize(dxBase_);

	// オーディオ初期化
	Audio::GetInstance()->Initialize();
}

void Framework::Finalize() {
	CloseHandle(dxBase_->GetFenceEvent());

	// キー入力処理解放
	Input::GetInstance()->Finalize();

	// スプライト共通部の解放
	SpriteCommon::GetInstance()->Finalize();

	// 3dオブジェクト共通部の解放
	Object3dCommon::GetInstance()->Finalize();

	// テクスチャマネージャの終了
	TextureManager::GetInstance()->Finalize();

	// 3Dモデルマネージャの終了
	ModelManager::GetInstance()->Finalize();

	// SRVマネージャの解放
	srvManager_->Finalize();

	// ライトマネージャの解放
	LightManager::GetInstance()->Finalize();

	// DirectX解放
	dxBase_->Finalize();

	// WindowsAPIの終了処理
	winApp_->Finalize();

	// オーディオマネジャーの解放
	Audio::GetInstance()->Finalize();
}

void Framework::Update() {
	if (winApp_->ProcessMessage()) {
		endRequest_ = true;
	}

	// キー入力の更新
	Input::GetInstance()->Update();
}

void Framework::Run() {
	// ゲームの初期化
	Initialize();

	while (true) { // ゲームのループ
		// 毎フレーム更新
		Update();
		// 終了リクエストが来たら抜ける
		if (IsEndRequest()) {
			break;
		}
		// 描画
		Draw();
	}
	// ゲームの終了
	Finalize();
}
