#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>
#include <cassert>
#include <dbghelp.h>
#include <strsafe.h>
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include <vector>
#include <wrl.h>
#include <xaudio2.h>
#include <random>
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

#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "dxcompiler.lib")
#pragma comment(lib, "xaudio2.lib")

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;
using namespace chrono;
using namespace MathFunction;

// チャンクヘッダー
struct ChunkHeader {
	char id[4]; // チャンク毎のID
	int32_t size; // チャンクサイズ
};

// RIFFヘッダチャンク
struct RiffHeader {
	ChunkHeader chunk; // "RIFF"
	char type[4]; // "WAVE"
};

// FMTチャンク
struct FormatChunk {
	ChunkHeader chunk; // "fmt"
	WAVEFORMATEX fmt; // 波形フォーマット
};

// サウンドデータ
struct SoundData {
	// 波形フォーマット
	WAVEFORMATEX wfex;
	// バッファの先頭アドレス
	BYTE* pBuffer;
	// バッファのサイズ
	unsigned int bufferSize;
};

// ブレンドモード
enum BlendMode {
	//!< ブレンドなし
	kBlendModeNone,
	//!< 通常αブレンド。Src * SrcA + Dest * (1 - SrcA)
	kBlendModeNormal,
	//!< 加算。Src * SrcA + Dest * 1
	kBlendModeAdd,
	//!< 減算。Dest * 1 - Src * SrcA
	kBlendModeSubtract,
	//!< 乗算。Src * 0 + Dest * Src
	kBlendModeMultiply,
	//!< スクリーン。Src * (1 - Dest) + Dest * 1
	kBlendModeScreen,
	// 利用してはいけない
	kCountOfBlendMode,
};

struct CameraForGPU {
	Vector3 worldPosition;
};

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

SoundData SoundLoadWave(const char* filename) {
	// ファイル入力ストリームのインスタンス
	ifstream file;
	// .wavファイルをバイナリモードで開く
	file.open(filename, ios_base::binary);
	// ファイルオープン失敗を検出する
	assert(file.is_open());

	// RIFFヘッダーの読み込み
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	// ファイルがRIFFかチェック
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}
	// タイプがWAVEかチェック
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(0);
	}

	// Formatチャンクの読み込み
	FormatChunk format = {};
	// チャンクヘッダーの確認
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		assert(0);
	}

	// チャンク本体の読み込み
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);

	// Dataチャンクの読み込み
	ChunkHeader data;
	file.read((char*)&data, sizeof(data));
	// JUNKチャンクを検出した場合
	if (strncmp(data.id, "JUNK", 4) == 0) {
		// 読み取り位置をJUNKチャンクの終わりまで進める
		file.seekg(data.size, ios_base::cur);
		// 再読み込み
		file.read((char*)&data, sizeof(data));
	}

	if (strncmp(data.id, "data", 4) != 0) {
		assert(0);
	}

	// Dataチャンクのデータ部（波形データ）の読み込み
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	// Waveファイルを閉じる
	file.close();

	// returnするためのデータ
	SoundData soundData = {};

	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;

	return soundData;
}

// 音声データ解放
void SoundUnload(SoundData* soundData) {
	// バッファのメモリを解放
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

void SoundPlayWave(const ComPtr<IXAudio2>& xAudio2, const SoundData& soundData) {
	HRESULT result;

	// 波形フォーマットを基にSourceVoiceの生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	// 再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	// 波形データの再生
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	D3DResourceLeakChecker leakCheck;
	ComPtr<IDXGIFactory7> dcgiFactory;

	CoInitializeEx(0, COINIT_MULTITHREADED);
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
	srvManager = new SrvManager();
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

	Transform uvTransformSprite{
		{ 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
	};

	// Δtを設定
	const float kDeltaTime = 1.0f / 60.0f;

	// xAudio
	ComPtr<IXAudio2> xAudio2;
	IXAudio2MasteringVoice* masterVoice;

	HRESULT result;

	// XAudioエンジンのインスタンスを生成
	result = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	// マスターボイスを生成
	result = xAudio2->CreateMasteringVoice(&masterVoice);

	// 音声読み込み
	SoundData soundData1 = SoundLoadWave("resources/Alarm01.wav");
	// 音声再生
	SoundPlayWave(xAudio2.Get(), soundData1);

	// ブレンドモード
	static int currentBlend = kBlendModeNone;
	const char* blendMode[] = { "kBlendModeNone", "kBlendModeNormal", "kBlendModeAdd", "kBlendModeSubtract", "kBlendModeMultiply", "kBlendModeScreen" };

	// パーティクルが動くか
	uint32_t canUpdate = false;

	// コマンドリストを生成する
	ComPtr<ID3D12GraphicsCommandList> commandList = dxBase->GetCommandList();

	// ウィンドウの×ボタンが押されるまでループ
	while (true) {
		// Windowsのメッセージ処理
		if (winApp->ProcessMessage()) {
			// ゲームループを抜ける
			break;
		}

		// ゲームの処理
		//ImGui_ImplDX12_NewFrame();
		//ImGui_ImplWin32_NewFrame();
		//ImGui::NewFrame();

		//// 開発用UIの処理
		//ImGui::ShowDemoWindow();

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

		// スプライトの更新
		sprite->Update();

		// 開発用UIの処理
		//ImGui::ShowDemoWindow();

		// ImGuiの内部コマンドを生成する
		//ImGui::Render();

		// 描画前処理
		dxBase->PreDraw();

		srvManager->PreDraw();

		// 3Dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
		object3dCommon->DrawSetting();

		for (uint32_t i = 0; i < 2; i++) {
			// 3Dオブジェクトの描画
			object3d[i]->Draw();
		}

		particleManager->Draw();

		// 共通描画設定
		spriteCommon->DrawSetting();

		// スプライトの描画
		sprite->Draw();

		// 実際のcommandListのImGuiの描画コマンドを積む
		//ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());

		// 描画後処理
		dxBase->PostDraw();

		Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
		uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
		uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
		/*materialData->uvTransform = uvTransformMatrix;*/
	}

	CloseHandle(dxBase->GetFenceEvent());

	// WindowsAPIの終了処理
	winApp->Finalize();

	// WindowsAPI解放
	delete winApp;
	winApp = nullptr;

	// キー入力処理解放
	delete input;

	//// スプライトの解放
	delete sprite;

	//// スプライト共通部の解放
	delete spriteCommon;

	for (uint32_t i = 0; i < 2; i++) {
		// 3dオブジェクトの解放
		delete object3d[i];
	}

	// 3dオブジェクト共通部の解放
	delete object3dCommon;

	// テクスチャマネージャの終了
	textureManager->Finalize();

	// 3Dモデルマネージャの終了
	modelManager->Finalize();

	// パーティクルマネージャの終了
	particleManager->Finalize();

	// SRVマネージャの解放
	delete srvManager;

	// DirectX解放
	delete dxBase;

	// XAudio2解放
	xAudio2.Reset();
	// 音声データ解放
	SoundUnload(&soundData1);

	// ImGuiの終了処理
	/*ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();*/

	return 0;
}