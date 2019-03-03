#include "stdafx.h"
#include "Util.h"
#include "Pipe.h"
#include "Win32Error.h"

using namespace std;

HANDLE Util::CreateChildProcess(wstring const& commandLine, Pipe const& output)
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

void Util::ReadFromPipe(HANDLE hPipe, vector<char>& buffer)
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

bool Util::CharToUnicode(vector<char> const& input, DWORD encoding, wstring& output)
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