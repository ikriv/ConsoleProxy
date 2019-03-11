#include "stdafx.h"
#include "Win32Error.h"
#include "CommandLine.h"
#include "Pipe.h"
#include "Util.h"
#include "ShowOutputArgs.h"

using namespace std;

void ShowError(wstring const& s)
{
	MessageBoxW(NULL, s.c_str(), L"Error", MB_OK | MB_ICONERROR);
}

void Usage()
{
	ShowError(ShowOutputArgs::usageInfo());
}

int APIENTRY wWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR    lpCmdLine,
	int       nCmdShow)
{
	try
	{
		// Note: commandLine.argv(0) is the first argument, NOT the process name, because lpCmdLine does not contain the process name
		CommandLine commandLine(lpCmdLine); 
		ShowOutputArgs args;

		if (!args.Parse(commandLine))
		{
			Usage();
			return 1;
		}
		
		Pipe output(false, true);
		HANDLE hProcess = Util::CreateChildProcess(args.processToRun(), args.flags(), output);
		if (!hProcess) return 2;
		CloseHandle(hProcess);

		output.CloseWrite(); // we are not going to write to the output; if we keep this open, reads below will hang

		vector<char> outputBuffer;
		Util::ReadFromPipe(output.ReadHandle(), outputBuffer);

		wstring processOutput;
		DWORD codePage = (args.flags() & DETACHED_PROCESS) ? CP_ACP : CP_OEMCP; // yes, the code page is different depending on the flags, ouch
		if (!Util::CharToUnicode(outputBuffer, codePage, processOutput)) return 2;

		MessageBoxW(NULL, processOutput.c_str(), L"Output", MB_OK);
	}
	catch (Win32Error const& err)
	{
		ShowError(err.message());
		return 2;
	}

	return 0;
}