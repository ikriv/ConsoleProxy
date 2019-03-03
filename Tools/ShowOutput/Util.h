#pragma once

class Pipe;

class Util
{
public:
	static HANDLE CreateChildProcess(std::wstring const& commandLine, Pipe const& output);
	static void ReadFromPipe(HANDLE hPipe, std::vector<char>& buffer);
	static bool CharToUnicode(std::vector<char> const& input, DWORD encoding, std::wstring& output);
};
