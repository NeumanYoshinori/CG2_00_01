#include "Audio.h"
#include <cassert>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <StringUtility.h>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

using namespace std;
using namespace Microsoft::WRL;

unique_ptr<Audio> Audio::instance_ = nullptr;

Audio* Audio::GetInstance() {
	if (instance_ == nullptr) {
		// PassKeyを渡してインスタンス生成
		instance_ = make_unique<Audio>(ConstructorKey());
	}
	return instance_.get();
}

void Audio::Finalize() {
	HRESULT result; // 他と使いまわし可能

	// Windows Media Foundationの終了
	result = MFShutdown();
	assert(SUCCEEDED(result));

	// XAudio2解放
	xAudio2_.Reset();
}

void Audio::Initialize() {
	HRESULT result; // 他と使いまわし可能

	// Windows Media Foundationの初期化（ローカルファイル版）
	result = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	assert(SUCCEEDED(result));

	// XAudioエンジンのインスタンスを生成
	result = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);

	// マスターボイスを生成
	result = xAudio2_->CreateMasteringVoice(&masterVoice_);
}

Audio::SoundData Audio::SoundLoadFile(const string& filename) {
	// フルパスをワイド文字列に変換
	wstring filePathW = StringUtility::ConvertString(filename);
	HRESULT result;

	// SourceReader作成
	ComPtr<IMFSourceReader> pReader;
	result = MFCreateSourceReaderFromURL(filePathW.c_str(), nullptr, &pReader);
	assert(SUCCEEDED(result));

	// PCM形式にフォーマット指定する
	ComPtr<IMFMediaType> pPCMType;
	MFCreateMediaType(&pPCMType);
	pPCMType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	pPCMType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	result = pReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, pPCMType.Get());
	assert(SUCCEEDED(result));

	// 実際にセットされたメディアタイプを取得する
	ComPtr<IMFMediaType> pOutType;
	pReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pOutType);

	// WAVEフォーマットを取得する
	WAVEFORMATEX* waveFormat = nullptr;
	MFCreateWaveFormatExFromMFMediaType(pOutType.Get(), &waveFormat, nullptr);

	// コンテナに格納する音声データ
	SoundData soundData = {};
	soundData.wfex = *waveFormat;

	// 生成したWaveフォーマットを解放
	CoTaskMemFree(waveFormat);

	// PCMデータのバッファを構築
	while (true) {
		ComPtr<IMFSample> pSample;
		DWORD streamIndex = 0, flags = 0;
		LONGLONG llTimeStamp = 0;
		// サンプルを読み込む
		result = pReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &streamIndex, &flags, &llTimeStamp, &pSample);
		// ストリームの末尾に達したら抜ける
		if (flags & MF_SOURCE_READERF_ENDOFSTREAM) break;
		if (pSample) {
			ComPtr<IMFMediaBuffer> pBuffer;
			// サンプルに含まれるサウンドデータ尾バッファを一繋ぎにして取得
			pSample->ConvertToContiguousBuffer(&pBuffer);

			BYTE* pData = nullptr;	// データ読み取り用ポインタ
			DWORD maxLength = 0, currentLength = 0;
			// バッファ読み込み用にロック
			pBuffer->Lock(&pData, &maxLength, &currentLength);
			// バッファの末尾にデータを追加
			soundData.buffer.insert(soundData.buffer.end(), pData, pData + currentLength);
			pBuffer->Unlock();
		}
	}

	return soundData;
}

// 音声データ解放
void Audio::SoundUnload(SoundData* soundData) {
	// バッファのメモリを解放
	soundData->buffer.clear();
	soundData->wfex = {};
}

IXAudio2SourceVoice* Audio::SoundPlayWave(const SoundData& soundData, bool loopFlag) {
	HRESULT result;

	// 波形フォーマットを基にSourceVoiceの生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	// 再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.buffer.data();
	buf.AudioBytes = static_cast<UINT32>(soundData.buffer.size());
	buf.Flags = XAUDIO2_END_OF_STREAM;

	if (loopFlag) {
		buf.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	// 波形データの再生
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();

	return pSourceVoice;
}

void Audio::SoundStopWave(IXAudio2SourceVoice* pSourceVoice) {
	pSourceVoice->Stop();
	pSourceVoice->FlushSourceBuffers();
	pSourceVoice->DestroyVoice();
}

void Audio::SoundPauseWave(IXAudio2SourceVoice* pSourceVoice) {
	pSourceVoice->Stop();
	pSourceVoice->DestroyVoice();
}
