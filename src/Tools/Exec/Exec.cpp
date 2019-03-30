// Exec.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\..\Common\ArgsUtil.h"

using namespace std;

static HANDLE hProcess = GetCurrentProcess();

void usage()
{
	puts("Usage: exec [-options] command");
	puts("Example: exec -arw cmd /c echo foo");
	puts("Options:");
	puts("\ta Set ANSI code page (CP_ACP)");
	puts("\tc Use CREATE_NEW_CONSOLE");
	puts("\td Use DETACHED_PROCESS");
	puts("\tr Explicitly redirect child's handles");
	puts("\tw Use CREATE_NO_WINDOW");
	puts("\t0 Use no special flags (default)");
	puts("");
	puts("c,d,w,0 are mutually exclusive (if several are specified, last wins");
}

HANDLE duplicate(HANDLE h)
{
	HANDLE result;
	if (!DuplicateHandle(hProcess, h, hProcess, &result, 0, TRUE, DUPLICATE_SAME_ACCESS)) return NULL;
	return result;
}

struct Options
{
	bool HasOptions;
	bool ForceAnsiCodepage;
	DWORD Flags;
	bool ForceRedirect;
};

bool getOptions(wchar_t* arg, Options* pOptions)
{
	if (*arg != '-') return true;
	pOptions->HasOptions = true;

	while (*++arg)
	{
		switch (*arg)
		{
		case 'a': pOptions->ForceAnsiCodepage = true; break;
		case 'c': pOptions->Flags = CREATE_NEW_CONSOLE; break;
		case 'd': pOptions->Flags = DETACHED_PROCESS; break;
		case 'r': pOptions->ForceRedirect = true; break;
		case 'w': pOptions->Flags = CREATE_NO_WINDOW; break;
		case '0': pOptions->Flags = 0; break;
		default: return false; // unknown option
		}
	}

	return true;
}

int wmain(int argc, wchar_t** argv)
{
	// argv[0] is the process name
	if (argc == 1)
	{
		usage();
		return 1;
	}

	DWORD flagsFromOption = 0;

	Options options = {};

	// check if argv[1] is an option
	if (argc > 2)
	{
		if (!getOptions(argv[1], &options)) return 1;
	}

	if (options.ForceAnsiCodepage)
	{
		SetConsoleOutputCP(GetACP());
	}

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	if (options.ForceRedirect)
	{
		si.dwFlags = STARTF_USESTDHANDLES;
		si.hStdInput = duplicate(GetStdHandle(STD_INPUT_HANDLE));
		si.hStdOutput = duplicate(GetStdHandle(STD_OUTPUT_HANDLE));
		si.hStdError = duplicate(GetStdHandle(STD_ERROR_HANDLE));
	}

	int skipArgs = options.HasOptions ? 2 : 1;
	int childArgc = argc - skipArgs;
	wchar_t** childArgv = argv + skipArgs;
	wstring command = ArgsUtil<wchar_t>::EscapeArgs(childArgc, childArgv);

	PROCESS_INFORMATION pi;

	if (!CreateProcessW(NULL, &command.front(), NULL, NULL, TRUE, options.Flags, NULL, NULL, &si, &pi))
	{
		DWORD error = GetLastError();
		return error;
	}

	CloseHandle(pi.hThread);
	WaitForSingleObject(pi.hProcess, INFINITE);

    return 0;
}

