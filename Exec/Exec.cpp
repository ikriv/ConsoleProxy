// Exec.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace std;

static HANDLE hProcess = GetCurrentProcess();

void usage()
{
	puts("Usage: exec command OR exec \"command arg1 arg2 ...\"");
}

HANDLE duplicate(HANDLE h)
{
	HANDLE result;
	if (!DuplicateHandle(hProcess, h, hProcess, &result, 0, TRUE, DUPLICATE_SAME_ACCESS)) return NULL;
	return result;
}

int wmain(int argc, wchar_t** argv)
{
	// argv[0] is the process name
	if (argc != 2)
	{
		usage();
		return ERROR_INVALID_DATA;
	}

	LPWSTR command = argv[1];
	DWORD processFlags = 0;

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	
	if (!GetConsoleWindow())
	{
		// There is no console, so we were started as either DETACHED_PROCESS or CREATE_NO_WINDOW.
		// This is important for handling non-ASCII output
		// If we are DETACHED_PROCESS, we will have system code page CP_ACP, CP 1251 for Cyrillic
		// If we are CREATE_NO_WINDOW, we will have OEM code page CP_OEMCP, CP 866 for Cyrillic
		// 
		// We want the child process to have the same code page as us, or the output will be garbled.
		// Thus, we run it detached if we are detached, and "no window" otherwise
		bool weAreDetached = GetConsoleOutputCP() == CP_ACP; 
		processFlags = weAreDetached ? DETACHED_PROCESS : CREATE_NO_WINDOW;

		si.dwFlags = STARTF_USESTDHANDLES;
		si.hStdInput = duplicate(GetStdHandle(STD_INPUT_HANDLE));
		si.hStdOutput = duplicate(GetStdHandle(STD_OUTPUT_HANDLE));
		si.hStdError = duplicate(GetStdHandle(STD_ERROR_HANDLE));
	}

	PROCESS_INFORMATION pi;

	if (!CreateProcessW(NULL, command, NULL, NULL, TRUE, processFlags, NULL, NULL, &si, &pi))
	{
		DWORD error = GetLastError();
		return error;
	}

	CloseHandle(pi.hThread);
	WaitForSingleObject(pi.hProcess, INFINITE);

    return 0;
}

