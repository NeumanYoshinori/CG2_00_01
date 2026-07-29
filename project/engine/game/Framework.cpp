#include "Framework.h"
#include "CrashHandler.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include "Object3dCommon.h"
#include "ModelManager.h"
#include "Audio.h"
#include "LightManager.h"
#include "RenderTextureCommon.h"
#include "PostEffect.h"
#include "Logger.h"
#include "ImGuiManager.h"
#include "ParticleManager.h"
#include "SkyboxCommon.h"

void Framework::Initialize() {
	// 誰も補足しなかった場合に(Unhandled)、補足する関数を登録
	SetUnhandledExceptionFilter(CrashHandler::ExportDump);

	// ログの生成
	Logger::GenerateLog();

	// WindowsAPIの初期化
	WinApp::GetInstance()->Initialize();

	// DirectXの初期化
	DirectXBase::GetInstance()->Initialize();

	// 入力の初期化
	Input::GetInstance()->Initialize();

	// SRVマネージャの初期化
	SrvManager::GetInstance()->Initialize();

	// テクスチャマネージャの初期化
	TextureManager::GetInstance()->Initialize();

	// スプライト共通部の初期化
	SpriteCommon::GetInstance()->Initialize();

	// ライトマネージャの初期化
	LightManager::GetInstance()->Initialize();

	// 3Dオブジェクト共通部の初期化
	Object3dCommon::GetInstance()->Initialize();

	// パーティクルマネージャ
	ParticleManager::GetInstance()->Initialize();

	// スカイボックス共通部の初期化
	SkyboxCommon::GetInstance()->Initialize();

	// レンダーテクスチャ基盤部分の初期化
	RenderTextureCommon::GetInstance()->Initialize();

	// ポストエフェクトの初期化
	PostEffect::GetInstance()->Initialize();

	// ImGuiマネージャの初期化
	ImGuiManager::GetInstance()->Initialize();

	// オーディオの初期化
	Audio::GetInstance()->Initialize();

	// シーンマネージャのインスタンス取得
	sceneManager_ = SceneManager::GetInstance();
}

void Framework::Finalize() {
	CloseHandle(DirectXBase::GetInstance()->GetFenceEvent());

	// スプライト共通部の解放
	SpriteCommon::GetInstance()->Finalize();

	// 3dオブジェクト共通部の解放
	Object3dCommon::GetInstance()->Finalize();

	// テクスチャマネージャの終了
	TextureManager::GetInstance()->Finalize();

	// 3Dモデルマネージャの終了
	ModelManager::GetInstance()->Finalize();

	// パーティクルマネージャの終了
	ParticleManager::GetInstance()->Finalize();

	// ImGuiマネージャの解放
	ImGuiManager::GetInstance()->Finalize();

	// ポストエフェクトの解放
	PostEffect::GetInstance()->Finalize();

	// レンダーテクスチャ基盤部分の解放
	RenderTextureCommon::GetInstance()->Finalize();

	// SRVマネージャの解放
	SrvManager::GetInstance()->Finalize();

	// ライトマネージャの解放
	LightManager::GetInstance()->Finalize();

	// スカイボックス共通部の解放
	SkyboxCommon::GetInstance()->Finalize();

	// DirectX解放
	DirectXBase::GetInstance()->Finalize();

	// WindowsAPIの終了処理
	WinApp::GetInstance()->Finalize();

	// シーンマネージャの解放
	sceneManager_->Finalize();

	// オーディオマネジャーの解放
	Audio::GetInstance()->Finalize();
}

void Framework::Update() {
	if (WinApp::GetInstance()->ProcessMessage()) {
		endRequest_ = true;
	}

	// キー入力の更新
	Input::GetInstance()->Update();

	// シーンマネージャの更新
	sceneManager_->Update();
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
