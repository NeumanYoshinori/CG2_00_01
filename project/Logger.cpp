#include "Logger.h"
#include <dinput.h>

using namespace std;

namespace Logger {
	void Log(const std::string& message){
		OutputDebugStringA(message.c_str());
	}
	void Log(ostream&os, const string& message) {
		os << message << endl;
		OutputDebugStringA(message.c_str());
	}
}
