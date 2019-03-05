// Exec.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ArgsUtil.h"

using namespace std;

static HANDLE hProcess = GetCurrentProcess();

void usage()
{
	puts("Usage: exec [options] command");
	puts("If command contain spaces, enclose it in quotes");
	puts("Options:");
	puts("\t-c CREATE_NEW_CONSOLE");
	puts("\t-d DETACHED_PROCESS");
	puts("\t-w CREATE_NO_WINDOW");
	puts("\t-0 no flags");
	puts("");
	puts("Without any options an intelligent default is used to ensure proper output redirection");
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
	bool hasOption = false;
	DWORD flagsFromOption = 0;

	// check if argv[1] is an option
	if (argc > 2)
	{
		hasOption = true;
		wchar_t* option = argv[1];
		if (wcscmp(L"-c", option) == 0) flagsFromOption = CREATE_NEW_CONSOLE;
		else if (wcscmp(L"-d", option) == 0) flagsFromOption = DETACHED_PROCESS;
		else if (wcscmp(L"-w", option) == 0) flagsFromOption = CREATE_NO_WINDOW;
		else if (wcscmp(L"-0", option) == 0) flagsFromOption = 0;
		else
		{
			hasOption = false;
		}
	}

	DWORD processFlags = flagsFromOption;

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	
	if (!hasOption && !GetConsoleWindow())
	{
		// Process flags option not specified and there is no console
		// Current process is started with either DETACHED_PROCESS or CREATE_NO_WINDOW.
		// The distinction is important for handling non-ASCII output
		// If we are DETACHED_PROCESS, we will have system code page CP_ACP, CP 1251 for Cyrillic
		// If we are CREATE_NO_WINDOW, we will have OEM code page CP_OEMCP, CP 866 for Cyrillic
		// 
		// We want the child process to have the same I/O handles and the same code page as us, or the output will be garbled.
		// Thus, we run it detached if we are detached, and "no window" otherwise
		bool weAreDetached = GetConsoleOutputCP() == CP_ACP; 
		processFlags = weAreDetached ? DETACHED_PROCESS : CREATE_NO_WINDOW;

		si.dwFlags = STARTF_USESTDHANDLES;
		si.hStdInput = duplicate(GetStdHandle(STD_INPUT_HANDLE));
		si.hStdOutput = duplicate(GetStdHandle(STD_OUTPUT_HANDLE));
		si.hStdError = duplicate(GetStdHandle(STD_ERROR_HANDLE));
	}

	int skipArgs = hasOption ? 2 : 1;
	int childArgc = argc - skipArgs;
	wchar_t** childArgv = argv + skipArgs;
	wstring command = ArgsUtil<wchar_t>::EscapeArgs(childArgc, childArgv);

	PROCESS_INFORMATION pi;

	if (!CreateProcessW(NULL, &command.front(), NULL, NULL, TRUE, processFlags, NULL, NULL, &si, &pi))
	{
		DWORD error = GetLastError();
		return error;
	}

	CloseHandle(pi.hThread);
	WaitForSingleObject(pi.hProcess, INFINITE);

    return 0;
}

