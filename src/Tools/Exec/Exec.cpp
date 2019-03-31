#include "stdafx.h"
#include "..\..\Common\ArgsUtil.h"

using namespace std;

static HANDLE hProcess = GetCurrentProcess();

#ifdef _CONSOLE
void message(string const& text)
{
	puts(text.c_str());
}
#else
void message(string const& text)
{
	MessageBoxA(NULL, text.c_str(), "ConsoleProxy", MB_OK | MB_ICONINFORMATION);
}
#endif

void usage()
{
	message(
		"Usage: exec [-options] command\r\n"
		"Example: exec -arw cmd /c echo foo\r\n"
		"Options:\r\n"
		"\ta Set ANSI code page (CP_ACP)\r\n"
		"\tc Use CREATE_NEW_CONSOLE\r\n"
		"\td Use DETACHED_PROCESS\r\n"
		"\tp Pause on exit\r\n"
		"\tr Explicitly redirect child's handles\r\n"
		"\tw Use CREATE_NO_WINDOW\r\n"
		"\t0 Use no special flags (default)\r\n"
		"\r\n"
		"c,d,w,0 are mutually exclusive (if several are specified, last wins");
}

HANDLE duplicate(HANDLE h)
{
	HANDLE result;
	if (!DuplicateHandle(hProcess, h, hProcess, &result, 0, TRUE, DUPLICATE_SAME_ACCESS))
	{
		message("Failed to duplicate handle");
		return NULL;
	}

	return result;
}

struct Options
{
	bool HasOptions;
	bool ForceAnsiCodepage;
	DWORD Flags;
	bool ForceRedirect;
	bool PauseOnExit;
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
		case 'p': pOptions->PauseOnExit = true; break;
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

	STARTUPINFOW si;
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

	if (options.PauseOnExit)
	{
#ifdef _CONSOLE
		message("Press any key to continue...");
		getchar();
#else
		message("Press OK to continue...");
#endif
	}

    return 0;
}

#ifndef _CONSOLE
int __stdcall wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, wchar_t* lpCmdLine, int nCmdShow)
{
	return wmain(__argc, __wargv);
}
#endif