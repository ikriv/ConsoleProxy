#include <Windows.h>
#include <string>
#include <sstream>
#include <vector>

#include "Win32Error.h"
#include "CommandLine.h"
#include "Pipe.h"
#include "Util.h"

using namespace std;

void ShowError(wstring const& s)
{
	MessageBoxW(NULL, s.c_str(), L"Error", MB_OK | MB_ICONERROR);
}

void Usage()
{
	ShowError(L"Usage: ShowOutput [options] what_to_run\r\n"
	          L"Options:\r\n"
		      L"\t-d, --detached: use DETACHED_PROCESS\r\n"
	          L"\t-n --no-window: use CREATE_NO_WINDOW\r\n");
}

int APIENTRY wWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR    lpCmdLine,
	int       nCmdShow)
{
	try
	{
		CommandLine commandLine(lpCmdLine);
		if (commandLine.size() == 0)
		{
			Usage();
			return 1;
		}
		
		Pipe output(false, true);
		HANDLE hProcess = Util::CreateChildProcess(commandLine.argv(0), output);
		if (!hProcess) return 2;
		CloseHandle(hProcess);

		output.CloseWrite(); // we are not going to write to the output; if we keep this open, reads below will hang

		vector<char> outputBuffer;
		Util::ReadFromPipe(output.ReadHandle(), outputBuffer);

		wstring processOutput;
		if (!Util::CharToUnicode(outputBuffer, CP_OEMCP, processOutput)) return 2;

		MessageBoxW(NULL, processOutput.c_str(), L"Output", MB_OK);
	}
	catch (Win32Error const& err)
	{
		ShowError(err.message());
		return 2;
	}

	return 0;
}