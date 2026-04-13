#include <Windows.h>
#include <filesystem>
#include <chrono>
#include <cassert>
#include <dbghelp.h>
#include <strsafe.h>
#include "Input.h"
#include "WinApp.h"
#include "D3DResourceLeakChecker.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "MathFunction.h"
#include "TextureManager.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "ModelManager.h"
#include "SrvManager.h"
#include "ParticleManager.h"
#include "ParticleEmitter.h"
#include "ImGuiManager.h"
#include "AudioManager.h"
#include "Sphere.h"

#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "dxcompiler.lib")

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;
using namespace chrono;
using namespace MathFunction;

static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception) {
	// 時刻を取得して、時刻を名前に入れたファイルを作成。Dumpsディレクトリ以下に出力
	SYSTEMTIME time;
	GetLocalTime(&time);
	wchar_t filePath[MAX_PATH] = { 0 };
	CreateDirectory(L"./Dumps", nullptr);
	StringCchPrintfW(filePath, MAX_PATH, L"./Dumps/%04d-%02d%02d-%02d%02d.dmp", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);
	HANDLE dumpFileHandle = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
	// processId（このexeのId）とクラッシュ（例外）の発生したthreadIdを取得
	DWORD processId = GetCurrentProcessId();
	DWORD threadId = GetCurrentThreadId();
	// 設定情報を入力
	MINIDUMP_EXCEPTION_INFORMATION minidumpInformation{ 0 };
	minidumpInformation.ThreadId = threadId;
	minidumpInformation.ExceptionPointers = exception;
	minidumpInformation.ClientPointers = TRUE;
	// Dumpを出力。MiniDumpNormalは最低限の情報を出力するフラグ
	MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle, MiniDumpNormal, &minidumpInformation, nullptr, nullptr);
	// ほかに関連づけられているSEH例外ハンドラがあれば実行。通常はプロセスを終了する

	return EXCEPTION_EXECUTE_HANDLER;
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	D3DResourceLeakChecker leakCheck;
	ComPtr<IDXGIFactory7> dcgiFactory;

	// 誰も補足しなかった場合に(Unhandled)、補足する関数を登録
	SetUnhandledExceptionFilter(ExportDump);

	// log出力用のフォルダ「logs」を作成
	filesystem::create_directory("logs");

	// ここからファイルを作成しofstreamを取得する
	// 現在時刻を取得
	system_clock::time_point now = system_clock::now();
	// 削って秒にする
	time_point<system_clock, seconds>
		nowSeconds = time_point_cast<seconds>(now);
	// 日本時間に変換
	zoned_time localTime{ current_zone(), nowSeconds };
	// formatを使って年月日_時分秒の文字列に変換
	string dateString = format("{:%Y%m%d_%H%M%S}", localTime);
	// 時刻を使ってファイル名を決定
	string logFilePath = string("logs/") + dateString + ".log";
	// ファイルを作って書き込み準備
	ofstream logStream(logFilePath);

	// ポインタ
	WinApp* winApp = nullptr;
	// WindowsAPIの初期化
	winApp = new WinApp();
	winApp->Initialize();

	// ポインタ
	DirectXBase* dxBase = nullptr;
	// DirectXの初期化
	dxBase = new DirectXBase();
	dxBase->Initialize(winApp);

	// ポインタ
	Input* input = nullptr;
	// 入力の初期化
	input = new Input();
	input->Initialize(winApp);

	SrvManager* srvManager = nullptr;
	// SRVマネージャの初期化
	srvManager = SrvManager::GetInstance();
	srvManager->Initialize(dxBase);

	// テクスチャマネジャー
	TextureManager* textureManager = TextureManager::GetInstance();

	// テクスチャマネージャの初期化
	textureManager->Initialize(dxBase, srvManager);

	// テクスチャを読み込む
	textureManager->LoadTexture("resources/uvChecker.png");
	textureManager->LoadTexture("resources/monsterBall.png");

	string filePath[2] = { "resources/uvChecker.png", "resources/monsterBall.png" };

	SpriteCommon* spriteCommon = nullptr;
	// スプライト共通部の初期化
	spriteCommon = new SpriteCommon();
	spriteCommon->Initialize(dxBase);

	// スプライトの初期化
	Sprite* sprite = new Sprite();
	sprite->Initialize(spriteCommon, "resources/uvChecker.png");

	// モデルマネージャー
	ModelManager* modelManager = ModelManager::GetInstance();

	// 3Dモデルマネージャの初期化
	modelManager->Initialize(dxBase);

	Object3dCommon* object3dCommon = nullptr;
	// 3Dオブジェクト共通部の初期化
	object3dCommon = new Object3dCommon();
	object3dCommon->Initialize(dxBase);

	Vector3 rotate[2] = { 0.0f };

	// .objファイルからモデルを読み込む
	ModelManager::GetInstance()->LoadModel("plane.obj");
	ModelManager::GetInstance()->LoadModel("axis.obj");

	// 3dオブジェクトの初期化
	Object3d* object3d[2];
	for (uint32_t i = 0; i < 2; i++) {
		object3d[i] = new Object3d();
		object3d[i]->Initialize(object3dCommon);
	}

	// 初期化済みの3Dオブジェクトにモデルを紐づける
	object3d[0]->SetModel("plane.obj");
	object3d[1]->SetModel("axis.obj");
	object3d[0]->SetTranslate({ 0.0f, 0.0f, 0.0f });
	object3d[1]->SetTranslate({ 2.0f, 2.0f, 2.0f });

	// カメラの初期化
	Camera* camera = new Camera();
	camera->SetRotate({ 0.3f, 0.0f, 0.0f });
	camera->SetTranslate({ 0.0f, 4.0f, -10.0f });
	object3d[0]->SetCamera(camera);
	object3d[1]->SetCamera(camera);

	// パーティクルマネージャ
	ParticleManager* particleManager = ParticleManager::GetInstance();
	particleManager->Initialize(dxBase, srvManager, camera);

	particleManager->CreateParticleGroup("circle", "resources/circle.png");

	Transform particleTransform;
	particleTransform.translate = { 0.0f, 0.0f, 0.0f };
	ParticleEmitter* particleEmitter = new ParticleEmitter("circle", particleTransform, 30, 1.0f);

	Sphere* sphere = new Sphere();
	sphere->Initialize(object3dCommon, "resources/monsterBall.png");
	sphere->SetCamera(camera);

	ImGuiManager* imGuiManager = new ImGuiManager();
	imGuiManager->Initialize(winApp, dxBase);

	// オーディオマネージャ初期化
	AudioManager* audioManager = new AudioManager();
	audioManager->Initialize();

	// 音声読み込み
	AudioManager::SoundData soundData1 = audioManager->SoundLoadFile("resources/Alarm01.wav");
	AudioManager::SoundData soundData2 = audioManager->SoundLoadFile("resources/The_maze_of_aqua.mp3");
	// 音声再生
	audioManager->SoundPlayWave(soundData2);

	// ウィンドウの×ボタンが押されるまでループ
	while (true) {
		// Windowsのメッセージ処理
		if (winApp->ProcessMessage()) {
			// ゲームループを抜ける
			break;
		}

		// キー入力の更新
		input->Update();

		// 0キーを押したときコンソールにHit 0と表示する
		if (input->ReleaseKey(DIK_0)) {
			OutputDebugStringA("Hit 0\n");
		}

		// カメラの更新
		camera->Update();

		rotate[0].x += 0.01f;
		rotate[1].z += 0.01f;

		for (uint32_t i = 0; i < 2; i++) {
			// 3Dオブジェクトの更新
			object3d[i]->Update();
			object3d[i]->SetRotate(rotate[i]);
		}

		particleManager->Update();

		particleEmitter->Update();

		// 球の更新
		sphere->Update();

		// スプライトの更新
		sprite->Update();

		imGuiManager->Begin();

#ifdef USE_IMGUI
		// デモウィンドウの表示オン
		ImGui::ShowDemoWindow();

		ImGui::Begin("Settings");
		camera->DebugUpdate();
		Vector2 spritePos = sprite->GetPosition();
		ImGui::DragFloat2("position", &spritePos.x);
		sprite->SetPosition(spritePos);
		ImGui::End();
#endif

		imGuiManager->End();

		// 描画前処理
		dxBase->PreDraw();

		srvManager->PreDraw();

		// 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
		object3dCommon->DrawSetting();

		for (uint32_t i = 0; i < 2; i++) {
			// 3Dオブジェクトの描画
			object3d[i]->Draw();
		}

		// 球の描画
		sphere->Draw();

		particleManager->Draw();

		// 共通描画設定
		spriteCommon->DrawSetting();

		// スプライトの描画
		sprite->Draw();

		imGuiManager->Draw();

		// 描画後処理
		dxBase->PostDraw();
	}

	CloseHandle(dxBase->GetFenceEvent());

	// キー入力処理解放
	delete input;

	// スプライトの解放
	delete sprite;

	// スプライト共通部の解放
	delete spriteCommon;

	for (uint32_t i = 0; i < 2; i++) {
		// 3dオブジェクトの解放
		delete object3d[i];
	}

	// 球の解放
	delete sphere;

	// 3dオブジェクト共通部の解放
	delete object3dCommon;

	// テクスチャマネージャの終了
	textureManager->Finalize();

	// 3Dモデルマネージャの終了
	modelManager->Finalize();

	// パーティクルマネージャの終了
	particleManager->Finalize();

	// SRVマネージャの解放
	srvManager->Finalize();

	// ImGuiマネージャの終了処理
	imGuiManager->Finalize();

	// ImGuiマネージャの解放
	delete imGuiManager;
	imGuiManager = nullptr;

	// DirectX解放
	delete dxBase;

	// WindowsAPIの終了処理
	winApp->Finalize();

	// WindowsAPI解放
	delete winApp;
	winApp = nullptr;

	// XAudio2解放
	audioManager->Finalize();
	// 音声データ開放
	audioManager->SoundUnload(&soundData1);

	// オーディオマネジャー解放
	delete audioManager;

	return 0;
}