#include "Logger.h"
#include <dinput.h>
#include <filesystem>
#include <fstream>
#include <chrono>

using namespace std;
using namespace chrono;

namespace Logger {
	void Log(const std::string& message){
		OutputDebugStringA(message.c_str());
	}
	void Log(ostream&os, const string& message) {
		os << message << endl;
		OutputDebugStringA(message.c_str());
	}
	void GenerateLog() {
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
		ofstream logStream(logFilePath); // ofstreamの型が完全になり、未初期化エラーも回避
	}
}
