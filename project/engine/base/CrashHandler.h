#pragma once
#include <Windows.h>
#include <dbghelp.h>
#include <strsafe.h>

#pragma comment(lib, "Dbghelp.lib")

namespace CrashHandler {
	LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception);
};

