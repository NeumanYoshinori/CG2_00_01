#pragma once
#include <Windows.h>
#include <xaudio2.h>
#include <wrl.h>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

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

	// サウンドデータ
	struct SoundData {
		// 波形フォーマット
		WAVEFORMATEX wfex{};
		// バッファ
		std::vector<BYTE> buffer;
	};

	// unique_ptr化したシングルトンインスタンス
	static std::unique_ptr<Audio> instance_;

	std::unordered_map<std::string, SoundData> soundDatas_;

	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
	IXAudio2MasteringVoice* masterVoice_ = nullptr;

	~Audio() = default;
	Audio(Audio&) = delete;
	Audio& operator=(Audio&) = delete;

	// default_delete にアクセスを許可する
	friend struct std::default_delete<Audio>;

public:
	// シングルトンインスタンスの取得
	static Audio* GetInstance();

	// コンストラクタに渡すための鍵
	class ConstructorKey {
	private:
		ConstructorKey() = default;
		friend class Audio;
	};

	// PassKeyを受け取るコンストラクタ
	explicit Audio(ConstructorKey) {}

	// 終了
	void Finalize();

	// 初期化
	void Initialize();

	// 音声ファイル読み込み
	void SoundLoadFile(const std::string& filename);

	// 音声データ解放
	void SoundUnload(const std::string& filename);

	// 音声再生
	IXAudio2SourceVoice* SoundPlayWave(const std::string& filename, bool loopFlag, float volume);

	void SoundStopWave(IXAudio2SourceVoice* pSourceVoice);

	void SoundPauseWave(IXAudio2SourceVoice* pSourceVoice);

	void SetVolume(IXAudio2SourceVoice* pSourceVoice, float volume);
};

