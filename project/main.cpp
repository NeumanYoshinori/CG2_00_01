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
#include "SkyboxCommon.h"
#include "Skybox.h"

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
	textureManager->LoadTexture("resources/rostock_laage_airport_4k.dds");

	// ファイルパス
	string filePath[2] = { "resources/uvChecker.png", "resources/monsterBall.png" };

	// スプライト共通部の初期化
	SpriteCommon* spriteCommon = new SpriteCommon();
	spriteCommon->Initialize(dxBase);

	// スプライトの初期化
	Sprite* sprite = new Sprite();
	sprite->Initialize(spriteCommon, "resources/uvChecker.png");

	// カメラの初期化
	Camera* camera = new Camera();
	camera->SetRotate({ 0.0f, 1.75f, 0.0f });
	camera->SetTranslate({ 0.0f, 0.0f, 0.0f });

	// ライトマネージャの初期化
	LightManager* lightManager = LightManager::GetInstance();
	lightManager->Initialize(dxBase);

	// モデルマネージャー
	ModelManager* modelManager = ModelManager::GetInstance();

	// 3Dモデルマネージャの初期化
	modelManager->Initialize(dxBase);

	// 3Dオブジェクト共通部の初期化
	Object3dCommon* object3dCommon = new Object3dCommon();
	object3dCommon->Initialize(dxBase);

	// .objファイルからモデルを読み込む
	ModelManager::GetInstance()->LoadModel("terrain.obj");

	// 3dオブジェクトの初期化
	Object3d* terrain = new Object3d();
	terrain->Initialize(object3dCommon);

	// 初期化済みの3Dオブジェクトにモデルを紐づける
	terrain->SetModel("terrain.obj");
	terrain->SetCamera(camera);

	// パーティクルマネージャ
	ParticleManager* particleManager = ParticleManager::GetInstance();
	particleManager->Initialize(dxBase, srvManager, camera);

	// 円のパーティクルグループを作成
	particleManager->CreateParticleGroup("circle", "resources/circle.png");

	Transform particleTransform;
	particleTransform.translate = { 0.0f, 0.0f, 0.0f };
	ParticleEmitter* particleEmitter = new ParticleEmitter("circle", particleTransform, 30, 1.0f);

	// 球の初期化
	Sphere* sphere = new Sphere();
	sphere->Initialize(object3dCommon, "resources/monsterBall.png");
	sphere->SetCamera(camera);

	// スカイボックス共通部の初期化
	SkyboxCommon* skyboxCommon = new SkyboxCommon();
	skyboxCommon->Initialize(dxBase);

	// スカイボックスの初期化
	Skybox* skybox = new Skybox();
	skybox->Initialize(skyboxCommon, "resources/rostock_laage_airport_4k.dds");
	skybox->SetCamera(camera);

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

		// スカイボックスの更新
		skybox->Update();

		// 3Dオブジェクトの更新
		terrain->Update();

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
		Vector3 terrainPos = terrain->GetTranslate();
		ImGui::DragFloat3("terrainPos", &terrainPos.x, 0.01f);
		terrain->SetTranslate(terrainPos);
		Vector3 terrainRot = terrain->GetRotate();
		ImGui::DragFloat3("terrainRot", &terrainRot.x, 0.01f);
		terrain->SetRotate(terrainRot);
		Vector3 spherePos = sphere->GetTranslate();
		ImGui::DragFloat3("spherePos", &spherePos.x, 0.01f);
		sphere->SetTranslate(spherePos);
		Vector3 skyboxPos = skybox->GetTranslate();
		ImGui::DragFloat3("skyboxPos", &skyboxPos.x, 0.01f);
		skybox->SetTranslate(skyboxPos);
		lightManager->DebugPointLight();
		ImGui::End();
#endif

		imGuiManager->End();

		// 描画前処理
		dxBase->PreDraw();

		srvManager->PreDraw();

		skyboxCommon->DrawSetting();

		skybox->Draw();

		// 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
		object3dCommon->DrawSetting();

		// 3Dオブジェクトの描画
		//terrain->Draw();

		// 球の描画
		//sphere->Draw();

		//particleManager->Draw();

		// 共通描画設定
		spriteCommon->DrawSetting();

		// スプライトの描画
		//sprite->Draw();

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

	// 3dオブジェクトの解放
	delete terrain;

	// 球の解放
	delete sphere;

	// 3dオブジェクト共通部の解放
	delete object3dCommon;

	// スカイボックスの解放
	delete skybox;

	// スカイボックス共通部の初期化
	delete skyboxCommon;

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

	// ライトマネージャの解放
	lightManager->Finalize();

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