#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>
#include <cassert>
#include <dbghelp.h>
#include <strsafe.h>
#include <dxgidebug.h>
#include "engine/math/Matrix.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"
#include <vector>
#include <wrl.h>
#include <xaudio2.h>
#include <random>
#include "Input.h"
#include "WinApp.h"
#include "DirectXBase.h"
#include "Logger.h"
#include "StringUtility.h"

#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "dxcompiler.lib")
#pragma comment(lib, "xaudio2.lib")

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;
using namespace chrono;
using namespace Logger;
using namespace StringUtility;

// 座標変換
struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

// 頂点データ
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

// マテリアル
struct Material {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
};

// 座標変換用行列
struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};

// 平行光源
struct DirectionalLight {
	Vector4 color;
	Vector3 direction;
	float intensity;
};

// マテリアルデータ
struct MaterialData {
	string textureFilePath;
};

// モデルデータ
struct ModelData {
	vector<VertexData> verticles;
	MaterialData material;
};

// リークチェッカー
struct D3DResourceLeakChecker {
	~D3DResourceLeakChecker()
	{
		// リソースリリースチェック
		ComPtr<IDXGIDebug1> debug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
			debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		}
	}
};

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

struct Particle {
	Transform transform;
	Vector3 velocity;
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

MaterialData LoadMaterialTemplateFile(const string& directoryPath, const string& filename) {
	// 中で必要となる変数の宣言
	MaterialData materialData; // 構築するMaterialData
	string line; // ファイルから読んだ1行を格納するもの
	ifstream file(directoryPath + "/" + filename); // ファイルを開く
	assert(file.is_open()); // とりあえず開けなかったら止める

	// 実際にファイルを読み、MaterialDataを構築していく
	while (getline(file, line)) {
		string identifier;
		istringstream s(line);
		s >> identifier;

		// identifierに応じた処理
		if (identifier == "map_Kd") {
			string textureFilename;
			s >> textureFilename;
			// 連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}

	// MaterialDataを返す
	return materialData;
}

ModelData LoadObjFile(const string& directoryPath, const string& filename) {
	// 中で必要となる変数の宣言
	ModelData modelData; // 構築するModelData
	vector<Vector4> positions; // 位置
	vector<Vector3> normals; // 法線
	vector<Vector2> texcoords; // テクスチャ座標
	string line; // ファイルから読んだ1行を格納するもの

	// ファイルを開く
	ifstream file(directoryPath + "/" + filename); // ファイルを開く
	assert(file.is_open()); // とりあえず開けなかったら止める

	// ファイルを読み、ModelDataを構築
	while (getline(file, line)) {
		string identifier;
		istringstream s(line);
		s >> identifier; // 先頭の識別子を読む

		// identifierに応じた
		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.x *= -1.0f;
			position.w = 1.0f;
			positions.push_back(position);
		}
		else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1.0f;
			normals.push_back(normal);
		}
		else if (identifier == "f") {
			VertexData triangle[3];
			// 面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				string vertexDefinition;
				s >> vertexDefinition;
				// 頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する
				istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (uint32_t element = 0; element < 3; ++element) {
					string index;
					getline(v, index, '/'); // 区切りでインデックスを読んでいく
					elementIndices[element] = stoi(index);
				}
				// 要素へのIndexから、実際の要素の値を取得して、頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				triangle[faceVertex] = { position, texcoord, normal };
			}
			// 頂点を逆順で登録することで、周り順を逆にする
			modelData.verticles.push_back(triangle[2]);
			modelData.verticles.push_back(triangle[1]);
			modelData.verticles.push_back(triangle[0]);
		}
		else if (identifier == "mtllib") {
			// materialTemplateLibraryファイルの名前を取得する
			string materialFilename;
			s >> materialFilename;
			// 基本的にobjファイルと同一改装にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}

	return modelData;
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

// パーティクル生成関数
Particle MakeNewParticle(mt19937& randomEngine) {
	uniform_real_distribution<float> distribution(-1.0f, 1.0f);
	Particle particle;
	particle.transform.scale = { 1.0f, 1.0f, 1.0f };
	particle.transform.rotate = { 0.0f, 0.0f, 0.0f };
	particle.transform.translate = { distribution(randomEngine), distribution(randomEngine), distribution(randomEngine) };
	particle.velocity = { distribution(randomEngine), distribution(randomEngine), distribution(randomEngine) };
	return particle;
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	D3DResourceLeakChecker leakCheck;
	ComPtr<IDXGIFactory7> dcgiFactory;
	ComPtr<ID3D12Device> device;

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
	Input* input = nullptr;

	// 入力の初期化
	input = new Input();
	input->Initialize(winApp);

	// ポインタ
	DirectXBase* dxBase = nullptr;

	// DirectXの初期化
	dxBase = new DirectXBase();
	dxBase->Initialize(winApp);

	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0; // 0から始まる
	descriptorRange[0].NumDescriptors = 1; // 数は1つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // Offsetを自動計算

	// RootParameter作成
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0; // レジスタ番号0とバインド
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // VertexShaderで使う
	rootParameters[1].Descriptor.ShaderRegister = 0; // レジスタ番号0とバインド
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTableを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange; // Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange); // Tableで利用する数
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	rootParameters[3].Descriptor.ShaderRegister = 1; // レジスタ番号1を使う
	descriptionRootSignature.pParameters = rootParameters; // ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters); // 配列の長さ

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0~1の範囲外をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // ありったけのMipmapを使う
	staticSamplers[0].ShaderRegister = 0; // レジスタ番号0を使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	HRESULT hr;
	// シリアライズしてバイナリにする
	ComPtr<ID3DBlob> signatureBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリを元に生成
	ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	hr = device->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	// RootSingature(パーティクル用)
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignatureForInstancing{};
	descriptionRootSignatureForInstancing.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRangeForInstancing[1] = {};
	descriptorRangeForInstancing[0].BaseShaderRegister = 0; // 0から始まる
	descriptorRangeForInstancing[0].NumDescriptors = 1; // 数は1つ
	descriptorRangeForInstancing[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う
	descriptorRangeForInstancing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// RootParameter作成
	D3D12_ROOT_PARAMETER rootParametersForInstancing[4] = {};
	rootParametersForInstancing[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う
	rootParametersForInstancing[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	rootParametersForInstancing[0].Descriptor.ShaderRegister = 0; // レジスタ番号0とバインド
	rootParametersForInstancing[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTableを使う
	rootParametersForInstancing[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // VertexShaderで使う
	rootParametersForInstancing[1].DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing; // Tableの中身の配列を指定
	rootParametersForInstancing[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstancing); // Tableで利用する数
	rootParametersForInstancing[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTableを使う
	rootParametersForInstancing[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	rootParametersForInstancing[2].DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing; // Tableの中身の配列を指定
	rootParametersForInstancing[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstancing); // Tableで利用する数
	rootParametersForInstancing[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う
	rootParametersForInstancing[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	rootParametersForInstancing[3].Descriptor.ShaderRegister = 1; // レジスタ番号1を使う
	descriptionRootSignatureForInstancing.pParameters = rootParametersForInstancing; // ルートパラメータ配列へのポインタ
	descriptionRootSignatureForInstancing.NumParameters = _countof(rootParametersForInstancing); // 配列の長さ

	descriptionRootSignatureForInstancing.pStaticSamplers = staticSamplers;
	descriptionRootSignatureForInstancing.NumStaticSamplers = _countof(staticSamplers);

	hr = D3D12SerializeRootSignature(&descriptionRootSignatureForInstancing,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリを元に生成
	ComPtr<ID3D12RootSignature> rootSignatureForInstancing = nullptr;
	hr = device->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignatureForInstancing));
	assert(SUCCEEDED(hr));

	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	// すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

	// RasiterzerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面（時計回り）を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	// Shaderをコンパイルする
	ComPtr<IDxcBlob> vertexShaderBlob = dxBase->CompileShader(L"resources/shaders/Object3D.VS.hlsl",
		L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	ComPtr<IDxcBlob> pixelShaderBlob = dxBase->CompileShader(L"resources/shaders/Object3D.PS.hlsl",
		L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	ComPtr<IDxcBlob> vertexShaderBlobForInstancing = dxBase->CompileShader(L"resources/shaders/Particle.VS.hlsl",
		L"vs_6_0");
	assert(vertexShaderBlobForInstancing != nullptr);

	ComPtr<IDxcBlob> pixelShaderBlobForInstancing = dxBase->CompileShader(L"resources/shaders/Particle.PS.hlsl",
		L"ps_6_0");
	assert(pixelShaderBlobForInstancing != nullptr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get(); // RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc; // InputLayout
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() }; // VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() }; // PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc; // BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc; // RasterizerState
	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ（形状）のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// 実際に生成
	ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDescForInstancing{};
	graphicsPipelineStateDescForInstancing.pRootSignature = rootSignatureForInstancing.Get(); // RootSignature
	graphicsPipelineStateDescForInstancing.InputLayout = inputLayoutDesc; // InputLayout
	graphicsPipelineStateDescForInstancing.VS = { vertexShaderBlobForInstancing->GetBufferPointer(),
	vertexShaderBlobForInstancing->GetBufferSize() }; // VertexShader
	graphicsPipelineStateDescForInstancing.PS = { pixelShaderBlobForInstancing->GetBufferPointer(),
	pixelShaderBlobForInstancing->GetBufferSize() }; // PixelShader
	graphicsPipelineStateDescForInstancing.BlendState = blendDesc; // BlendState
	graphicsPipelineStateDescForInstancing.RasterizerState = rasterizerDesc; // RasterizerState
	// 書き込むRTVの情報
	graphicsPipelineStateDescForInstancing.NumRenderTargets = 1;
	graphicsPipelineStateDescForInstancing.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ（形状）のタイプ。三角形
	graphicsPipelineStateDescForInstancing.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定
	graphicsPipelineStateDescForInstancing.SampleDesc.Count = 1;
	graphicsPipelineStateDescForInstancing.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// DepthStencilの設定
	graphicsPipelineStateDescForInstancing.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDescForInstancing.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// 実際に生成
	ComPtr<ID3D12PipelineState> graphicsPipelineStateForInstancing = nullptr;
	hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDescForInstancing,
		IID_PPV_ARGS(&graphicsPipelineStateForInstancing));
	assert(SUCCEEDED(hr));

	// モデル読み込み
	ModelData modelData;
	modelData.verticles.push_back({ .position = {1.0f, 1.0f, 0.0f, 1.0f}, .texcoord = {0.0f, 0.0f}, .normal = {0.0f, 0.0f, 1.0f} });
	modelData.verticles.push_back({ .position = {-1.0f, 1.0f, 0.0f, 1.0f}, .texcoord = {1.0f, 0.0f}, .normal = {0.0f, 0.0f, 1.0f} });
	modelData.verticles.push_back({ .position = {1.0f, -1.0f, 0.0f, 1.0f}, .texcoord = {0.0f, 1.0f}, .normal = {0.0f, 0.0f, 1.0f} });
	modelData.verticles.push_back({ .position = {1.0f, -1.0f, 0.0f, 1.0f}, .texcoord = {0.0f, 1.0f}, .normal = {0.0f, 0.0f, 1.0f} });
	modelData.verticles.push_back({ .position = {-1.0f, 1.0f, 0.0f, 1.0f}, .texcoord = {1.0f, 0.0f}, .normal = {0.0f, 0.0f, 1.0f} });
	modelData.verticles.push_back({ .position = {-1.0f, -1.0f, 0.0f, 1.0f}, .texcoord = {1.0f, 1.0f}, .normal = {0.0f, 0.0f, 1.0f} });
	modelData.material.textureFilePath = "./resources/uvChecker.png";
	// 頂点リソースを作る
	ComPtr<ID3D12Resource> vertexResource = dxBase->CreateBufferResource(sizeof(VertexData) * modelData.verticles.size());
	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点のサイズ
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.verticles.size());
	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// 頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	// 書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	memcpy(vertexData, modelData.verticles.data(), sizeof(VertexData) * modelData.verticles.size());

	// 数学関数
	Matrix* matrix = new Matrix;

	const uint32_t kNumInstance = 10; // インスタンス数
	// Instancing用のTransformationMatrixリソースを作る
	ComPtr<ID3D12Resource> instancingResource =
		dxBase->CreateBufferResource(sizeof(TransformationMatrix) * kNumInstance);
	// 書き込むためのアドレスを取得
	TransformationMatrix* instancingData = nullptr;
	instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&instancingData));
	// 単位行列を書き込んでおく
	for (uint32_t index = 0; index < kNumInstance; ++index) {
		instancingData[index].WVP = matrix->MakeIdentity4x4();
		instancingData[index].World = matrix->MakeIdentity4x4();
	}

	// Sprite用の頂点リソースを作る
	ComPtr<ID3D12Resource> vertexResourceSprite = dxBase->CreateBufferResource(sizeof(VertexData) * 6);

	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	// Sprite用の頂点リソースを作る
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	// 頂点バッファビューを作成する
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 6;
	// 1頂点あたりのサイズ
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

	VertexData* vertexDataSprite = nullptr;
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
	// 1枚目の三角形
	vertexDataSprite[0].position = { 0.0f, 360.0f, 0.0f, 1.0f }; // 左下
	vertexDataSprite[0].texcoord = { 0.0f, 1.0f };
	vertexDataSprite[1].position = { 0.0f, 0.0f, 0.0f, 1.0f }; // 左上
	vertexDataSprite[1].texcoord = { 0.0f, 0.0f };
	vertexDataSprite[2].position = { 640.0f, 360.0f, 0.0f, 1.0f }; // 右下
	vertexDataSprite[2].texcoord = { 1.0f, 1.0f };
	// 2枚目の三角形
	vertexDataSprite[3].position = { 640.0f, 0.0f, 0.0f, 1.0f }; // 左上
	vertexDataSprite[3].texcoord = { 1.0f, 0.0f };

	ComPtr<ID3D12Resource> indexResourceSprite = dxBase->CreateBufferResource(sizeof(uint32_t) * 6);
	D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	// リソースの先頭のアドレスから使う
	indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	// インデックスはuint32_tとする
	indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;

	// インデックスリソースを書き込む
	uint32_t* indexDataSprite = nullptr;
	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	indexDataSprite[0] = 0; indexDataSprite[1] = 1; indexDataSprite[2] = 2;
	indexDataSprite[3] = 1; indexDataSprite[4] = 3; indexDataSprite[5] = 2;

	// 乱数生成器の初期化
	random_device seedGenerator;
	mt19937 randomEngine(seedGenerator());

	uniform_real_distribution<float> distribution(-1.0f, 1.0f);

	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	ComPtr<ID3D12Resource> materialResource = dxBase->CreateBufferResource(sizeof(Material));
	// マテリアルにデータを書き込む
	Material* materialData = nullptr;
	// 書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	// 今回は赤を書き込んでみる
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->enableLighting = true;
	materialData->uvTransform = matrix->MakeIdentity4x4();

	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	ComPtr<ID3D12Resource> materialResourceSprite = dxBase->CreateBufferResource(sizeof(Material));
	// マテリアルにデータを書き込む
	Material* materialDataSprite = nullptr;
	// 書き込むためのアドレスを取得
	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	// 今回は赤を書き込んでみる
	materialDataSprite->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialDataSprite->enableLighting = false;
	materialDataSprite->uvTransform = matrix->MakeIdentity4x4();

	Transform uvTransformSprite{
		{ 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
	};

	ComPtr<ID3D12Resource> directionalLightResource = dxBase->CreateBufferResource(sizeof(DirectionalLight));
	DirectionalLight* directionalLightData = nullptr;
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	// デフォルト値はとりあえず以下のようにしておく
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;

	// WVP用のリソースを作る
	ComPtr<ID3D12Resource> wvpResource = dxBase->CreateBufferResource(sizeof(TransformationMatrix));

	Particle particles[kNumInstance];
	for (uint32_t index = 0; index < kNumInstance; ++index) {
		// 位置と速度を[-1,1]でランダムに初期化
		particles[index] = MakeNewParticle(randomEngine);
	}

	// Δtを設定
	const float kDeltaTime = 1.0f / 60.0f;

	Transform cameraTransform{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 4.0f, 10.0f} };

	// データを書き込む
	TransformationMatrix* wvpData = nullptr;
	// 書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	// 単位行列を書き込んでおく
	wvpData->WVP = matrix->MakeIdentity4x4();
	wvpData->World = matrix->MakeIdentity4x4();

	Transform transformSprite{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

	// Sprite用のTransformationMatrix用のリソースを作る。
	ComPtr<ID3D12Resource> wvpResourceSprite = dxBase->CreateBufferResource(sizeof(TransformationMatrix));
	// データを書き込む
	TransformationMatrix* wvpDataSprite = nullptr;
	// 書き込むためのアドレスを取得
	wvpResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&wvpDataSprite));
	// 単位行列を書き込んでおく
	wvpDataSprite->WVP = matrix->MakeIdentity4x4();
	wvpDataSprite->World = matrix->MakeIdentity4x4();

	// Texture
	ScratchImage mipImages = dxBase->LoadTexture("resources/uvChecker.png");
	const TexMetadata& metadata = mipImages.GetMetadata();
	ComPtr<ID3D12Resource> textureResource = dxBase->CreateTextureResource(metadata);
	ComPtr<ID3D12Resource> intermediateResource = dxBase->UploadTextureData(textureResource, mipImages);

	// 2枚目のTextureを読んで転送する
	ScratchImage mipImages2 = dxBase->LoadTexture(modelData.material.textureFilePath);
	const TexMetadata& metadata2 = mipImages2.GetMetadata();
	ComPtr<ID3D12Resource> textureResource2 = dxBase->CreateTextureResource(metadata2);
	ComPtr<ID3D12Resource> intermediateResource2 = dxBase->UploadTextureData(textureResource2, mipImages2);

	// metadataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	// SRVを作成するDescriptoHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = dxBase->GetSRVCPUDescriptorHandle(0);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = dxBase->GetSRVGPUDescriptorHandle(0);
	// 先頭はImGuiが使っているのでその次を使う
	textureSrvHandleCPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	// SRVの生成
	device->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);

	// metadataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metadata2.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

	// SRVを作成するDescriptoHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = dxBase->GetSRVCPUDescriptorHandle(2);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = dxBase->GetSRVGPUDescriptorHandle(2);
	// SRVの生成
	device->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);

	D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};
	instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	instancingSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	instancingSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	instancingSrvDesc.Buffer.FirstElement = 0;
	instancingSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	instancingSrvDesc.Buffer.NumElements = kNumInstance;
	instancingSrvDesc.Buffer.StructureByteStride = sizeof(TransformationMatrix);
	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU = dxBase->GetSRVCPUDescriptorHandle(3);
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU = dxBase->GetSRVGPUDescriptorHandle(3);
	device->CreateShaderResourceView(instancingResource.Get(), &instancingSrvDesc, instancingSrvHandleCPU);

	// モンスターボールを使うか
	bool useMonsterBall = true;

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

	// ウィンドウの×ボタンが押されるまでループ
	while (true) {
		// Windowsのメッセージ処理
		if (winApp->ProcessMessage()) {
			// ゲームループを抜ける
			break;
		}

		// ゲームの処理
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// 開発用UIの処理
		ImGui::ShowDemoWindow();

		switch (currentBlend) {
		case kBlendModeNone:
			blendDesc.RenderTarget[0].BlendEnable = FALSE;

			break;
		case kBlendModeNormal:
			blendDesc.RenderTarget[0].BlendEnable = TRUE;
			blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

			break;
		case kBlendModeAdd:
			blendDesc.RenderTarget[0].BlendEnable = TRUE;
			blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;

			break;
		case kBlendModeSubtract:
			blendDesc.RenderTarget[0].BlendEnable = TRUE;
			blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
			blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;

			break;
		case kBlendModeMultiply:
			blendDesc.RenderTarget[0].BlendEnable = TRUE;
			blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
			blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;

			break;
		case kBlendModeScreen:
			blendDesc.RenderTarget[0].BlendEnable = TRUE;
			blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
			blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;

			break;
		}

		graphicsPipelineStateDesc.BlendState = blendDesc; // BlendState
		hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
			IID_PPV_ARGS(&graphicsPipelineState));
		assert(SUCCEEDED(hr));

		graphicsPipelineStateDescForInstancing.BlendState = blendDesc; // BlendState
		hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDescForInstancing,
			IID_PPV_ARGS(&graphicsPipelineStateForInstancing));
		assert(SUCCEEDED(hr));

		input->Update();

		if (input->ReleaseKey(DIK_0)) {
			OutputDebugStringA("Hit 0\n");
		}

		if (canUpdate) {
			for (uint32_t index = 0; index < kNumInstance; index++) {
				particles[index].transform.translate += particles[index].velocity * kDeltaTime;
			}
		}

		// 開発用UIの処理
		ImGui::ShowDemoWindow();

		ImGui::Begin("Setting");
		ImGui::DragFloat3("cameraTranslate", &cameraTransform.translate.x, 0.01f);
		ImGui::SliderAngle("CameraRotateX", &cameraTransform.rotate.x, 0.01f);
		ImGui::SliderAngle("CameraRotateY", &cameraTransform.rotate.y, 0.01f);
		ImGui::SliderAngle("CameraRotateZ", &cameraTransform.rotate.z, 0.01f);
		ImGui::SliderAngle("SphereRotateX", &particles[0].transform.rotate.x, 0.01f);
		ImGui::SliderAngle("SphereRotateY", &particles[0].transform.rotate.y, 0.01f);
		ImGui::SliderAngle("SphereRotateZ", &particles[0].transform.rotate.z, 0.01f);
		ImGui::ColorEdit4("color", &materialData->color.x);
		ImGui::CheckboxFlags("enableLighting", &materialData->enableLighting, 1);
		ImGui::CheckboxFlags("update", &canUpdate, 1);
		if (ImGui::BeginCombo("Blend", blendMode[currentBlend])) {
			for (uint32_t i = 0; i < 6; ++i) {
				const bool isSelected = (currentBlend == i);
				if (ImGui::Selectable(blendMode[i], isSelected)) {
					currentBlend = i;

					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
			}
			ImGui::EndCombo();
		}
		ImGui::Checkbox("useMonsterBall", &useMonsterBall);
		ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
		ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);
		ImGui::End();

		// ImGuiの内部コマンドを生成する
		ImGui::Render();

		// 描画前処理
		dxBase->PreDraw();

		// コマンドリストを生成する
		ComPtr<ID3D12GraphicsCommandList> commandList = dxBase->GetCommandList();

		// RootSignatureを設定。PSOに設定しているけど別途設定が必要
		commandList->SetGraphicsRootSignature(rootSignatureForInstancing.Get());
		commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
		commandList->SetPipelineState(graphicsPipelineStateForInstancing.Get());
		// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Modelの描画
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
		// マテリアルCBufferの場所を設定
		commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
		// instancing用のDataを読み込むためにStructuredBufferのSRVを設定する
		commandList->SetGraphicsRootDescriptorTable(1, instancingSrvHandleGPU);
		// SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
		commandList->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);
		// 描画
		commandList->DrawInstanced(UINT(modelData.verticles.size()), kNumInstance, 0, 0);

		commandList->IASetIndexBuffer(&indexBufferViewSprite); // IBVを設定
		// RootSignatureを設定
		commandList->SetGraphicsRootSignature(rootSignature.Get());
		commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
		commandList->SetPipelineState(graphicsPipelineState.Get());

		// Spriteの描画
		commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSprite); // VBVを設定
		commandList->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
		commandList->SetGraphicsRootConstantBufferView(1, wvpResourceSprite->GetGPUVirtualAddress());
		commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
		// TransformationMatrixCBufferの場所を設定
		// 描画！（DrawCall/ドローコール）
		//commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);

		commandList->ClearDepthStencilView(dxBase->GetDsvHandle(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		// 実際のcommandListのImGuiの描画コマンドを積む
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());

		dxBase->PostDraw();

		// Model用のWVPMatrixを作る
		for (uint32_t index = 0; index < kNumInstance; ++index) {
			Matrix4x4 worldMatrix = matrix->MakeAffineMatrix(particles[index].transform.scale, particles[index].transform.rotate, particles[index].transform.translate);
			Matrix4x4 cameraMatrix = matrix->MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
			Matrix4x4 viewMatrix = matrix->Inverse(cameraMatrix);
			Matrix4x4 projectionMatrix = matrix->MakePerspectiveFovMatrix(0.45f, float(WinApp::kClientWidth) / float(WinApp::kClientHeight), 0.1f, 100.0f);
			Matrix4x4 wvpMatrix = matrix->Multiply(worldMatrix, matrix->Multiply(viewMatrix, projectionMatrix));
			instancingData[index] = { wvpMatrix, worldMatrix };
		}

		// Sprite用のWorldViewProjectionMatrixを作る
		Matrix4x4 worldMatrixSprite = matrix->MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
		Matrix4x4 viewMatrixSprite = matrix->MakeIdentity4x4();
		Matrix4x4 projectionMatrixSprite = matrix->MakeOrthographicMatrix(0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.0f, 100.0f);
		Matrix4x4 wvpMatrixSprite = matrix->Multiply(worldMatrixSprite, matrix->Multiply(viewMatrixSprite, projectionMatrixSprite));
		*wvpDataSprite = { wvpMatrixSprite, worldMatrixSprite };

		Matrix4x4 uvTransformMatrix = matrix->MakeScaleMatrix(uvTransformSprite.scale);
		uvTransformMatrix = matrix->Multiply(uvTransformMatrix, matrix->MakeRotateZMatrix(uvTransformSprite.rotate.z));
		uvTransformMatrix = matrix->Multiply(uvTransformMatrix, matrix->MakeTranslateMatrix(uvTransformSprite.translate));
		materialDataSprite->uvTransform = uvTransformMatrix;
	}

	CloseHandle(dxBase->GetFenceEvent());

	// WindowsAPIの終了処理
	winApp->Finalize();

	// WindowsAPI解放
	delete winApp;
	winApp = nullptr;

	// 数学関数解放
	delete matrix;
	// キー入力処理解放
	delete input;

	// DirectX解放
	delete dxBase;

	// XAudio2解放
	xAudio2.Reset();
	// 音声データ解放
	SoundUnload(&soundData1);

	// ImGuiの終了処理
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	return 0;
}