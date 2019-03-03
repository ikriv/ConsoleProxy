// Exec.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace std;

static HANDLE hProcess = GetCurrentProcess();

void usage()
{
	printf("Usage: exec command OR exec \"command arg1 arg2 ...\"");
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
		// we are running without a console, run the target without a window with redirected handles
		processFlags = CREATE_NO_WINDOW;
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

