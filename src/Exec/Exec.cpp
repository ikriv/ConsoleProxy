// Exec.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ArgsUtil.h"

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
	puts("\t0 Use no special flags");
	puts("");
	puts("c,d,w,0 are mutually exclusive");
	puts("Default options are -rw when Exec process itself is detached, and -0 in all other cases");
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
	DWORD flagsFromOption = 0;

	Options options = {};

	// check if argv[1] is an option
	if (argc > 2)
	{
		if (!getOptions(argv[1], &options)) return 1;
	}

	if (!options.HasOptions)
	{
		if (GetConsoleWindow() || GetConsoleOutputCP())
		{
			// we have a console, or were created using CREATE_NO_WINDOW, in which case GetConsoleOutputCP() is not 0
			options.Flags = 0;
		}
		else
		{
			// Plus: this automatically redirects output of grandchildren
			// Con: this sets wrong code page for international output.
			//
			// Normally console applications use OEM codepage for standard output (CP866 for Russian), but detached procseses use ANSI codepage (CP1251 for Russian).
			// Our parent invoked us as DETACHED_PROSESS, so it may expect the output in the system codepage (or may just not care).
			// Our child process, being run as CREATE_NO_WINDOW, will produce output in OEM codepage, and our parent may not decode it correctly.
			// If we had our own console, the child would inherit its encoding, but we are detached, and I don't know about a way to force ANSI code page on the child.
			//
			// Another sensible option would be to run the child detached ('-dr") as well. This ensures ANSI code page, but fails to redirect the output 
			// of the grandchild process, if any
			//
			// If we want to preserve the code page AND account for the grandchildren, we must use an extra process that would switch to ANSI code page: 
			// Exec -rw Exec -a child. However, this situation is rare. CREATE_NO_WINDOW+rediect would work just fine in most practical cases.

			options.Flags = CREATE_NO_WINDOW;
			options.ForceRedirect = true;
		}
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

