#include <Windows.h>
#include <string>
#include <sstream>
#include <vector>

#include "Win32Error.h"
#include "CommandLine.h"
#include "Pipe.h"

using namespace std;

void ShowError(wstring const& s)
{
	MessageBoxW(NULL, s.c_str(), L"Error", MB_OK | MB_ICONERROR);
}

HANDLE CreateChildProcess(wstring const& commandLine, Pipe const& output)
{
	STARTUPINFOW si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdOutput = output.WriteHandle();

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));
	DWORD flags = CREATE_NO_WINDOW; // DETACHED_PROCESS

	wstring commandLineCopy = commandLine; // writable copy

	BOOL bProcessCreated = CreateProcessW(NULL, &commandLineCopy.front(), NULL, NULL, TRUE, flags, NULL, NULL, &si, &pi);
	if (!bProcessCreated) throw Win32Error(L"CreateProcessW");

	CloseHandle(pi.hThread);
	return pi.hProcess;
}

void ReadFromPipe(HANDLE hPipe, vector<char>& buffer)
{
	const int CHUNK_SIZE = 2048;

	while (true)
	{
		size_t oldSize = buffer.size();
		size_t newSize = oldSize + CHUNK_SIZE;
		buffer.resize(newSize);

		// TODO: add better error handling
		DWORD bytesRead;
		BOOL bReadFileOk = ReadFile(hPipe, &buffer[oldSize], CHUNK_SIZE, &bytesRead, NULL) && bytesRead > 0;

		if (!bReadFileOk) bytesRead = 0;
		size_t totalSize = oldSize + bytesRead;
		buffer.resize(totalSize);

		if (!bReadFileOk) break;
	}
}

bool CharToUnicode(vector<char> const& input, DWORD encoding, wstring& output)
{
	output.clear();
	if (input.size() == 0) return true;

	DWORD outputSize = MultiByteToWideChar(encoding, 0, &input.front(), input.size(), NULL, 0);

	if (outputSize > 0)
	{
		output.resize(outputSize);
		outputSize = MultiByteToWideChar(encoding, 0, &input.front(), input.size(), &output.front(), outputSize);
	}

	if (outputSize == 0) throw Win32Error(L"MultiByteToWideChar");

	return true;
}

void Usage()
{
	ShowError(L"Usage: ShowOutput what_to_run");
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
		HANDLE hProcess = CreateChildProcess(commandLine.argv(0), output);
		if (!hProcess) return 2;
		CloseHandle(hProcess);

		output.CloseWrite(); // we are not going to write to the output; if we keep this open, reads below will hang

		vector<char> outputBuffer;
		ReadFromPipe(output.ReadHandle(), outputBuffer);

		wstring processOutput;
		if (!CharToUnicode(outputBuffer, CP_OEMCP, processOutput)) return 2;

		MessageBoxW(NULL, processOutput.c_str(), L"Output", MB_OK);
	}
	catch (Win32Error const& err)
	{
		ShowError(err.message());
		return 2;
	}

	return 0;
}