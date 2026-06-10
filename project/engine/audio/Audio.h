#pragma once
#include <Windows.h>
#include <xaudio2.h>
#include <fstream>
#include <wrl.h>
#include <vector>

#pragma comment(lib, "xaudio2.lib")

class Audio {
private:
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

	// インスタンス
	static Audio* instance;

	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
	IXAudio2MasteringVoice* masterVoice;

	Audio() = default;
	~Audio() = default;
	Audio(Audio&) = delete;
	Audio& operator=(Audio&) = delete;

public:
	// サウンドデータ
	struct SoundData {
		// 波形フォーマット
		WAVEFORMATEX wfex;
		// バッファ
		std::vector<BYTE> buffer;
	};

	// シングルトンインスタンスの取得
	static Audio* GetInstance();

	// 終了
	void Finalize();

	// 初期化
	void Initialize();

	// 後始末
	void Release();

	// 音声ファイル読み込み
	SoundData SoundLoadFile(const std::string& filename);

	// 音声データ解放
	void SoundUnload(SoundData* soundData);

	// 音声再生
	void SoundPlayWave(const SoundData& soundData, bool loop);
};

