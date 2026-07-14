#pragma once
#include <Windows.h>

namespace CrashHandler {
	LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception);
};

