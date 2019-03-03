#pragma once

class Pipe
{
	HANDLE hRead;
	HANDLE hWrite;

	static void Close(HANDLE* pHandle)
	{
		if (!pHandle || !*pHandle) return;
		CloseHandle(*pHandle);
		*pHandle = NULL;
	}

public:
	Pipe(bool inheritRead, bool inheritWrite)
	{
		hRead = hWrite = NULL;

		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.bInheritHandle = TRUE;
		sa.lpSecurityDescriptor = NULL;

		if (!CreatePipe(&hRead, &hWrite, &sa, 0)) throw Win32Error(L"CreatePipe");

		if (!inheritRead) SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);
		if (!inheritWrite) SetHandleInformation(hWrite, HANDLE_FLAG_INHERIT, 0);
	}

	~Pipe()
	{
		CloseRead();
		CloseWrite();
	}

	HANDLE ReadHandle() const { return hRead; }
	HANDLE WriteHandle() const { return hWrite; }
	void CloseRead() { Close(&hRead); }
	void CloseWrite() { Close(&hWrite); }
};

