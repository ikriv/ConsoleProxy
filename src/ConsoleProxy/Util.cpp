#include <Windows.h>
#include "Util.h"
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

string util::getEnvVar(string const& name)
{
	string result;

	// GetEnvironmentVariableA() returns size of required buffer, including terminating \0
	DWORD size = GetEnvironmentVariableA(name.c_str(), NULL, 0);
	if (!size) return result;
	result.resize(size);

	// GetEnvironmentVariableA() returns the number of characters written, NOT including terminating \0
	DWORD actual_size = GetEnvironmentVariableA(name.c_str(), &result.front(), size); // normally

	result.resize(actual_size);
	return result;
}

string util::getFileNameWithoutExtension(string const& path)
{
	int start = path.find_last_of('\\');
	if (start == string::npos) start = 0; else ++start;
	if (start >= (int)path.size()) return string();

	int end = path.find_last_of('.');
	if (end == string::npos || end <= start) end = path.size();

	return path.substr(start, end - start);
}

string util::getFormattedTimeUtc()
{
	SYSTEMTIME time;
	GetSystemTime(&time);
	ostringstream str;
	str.fill('0');

	str.width(4); str << time.wYear;
	str.width(2); str << time.wMonth << time.wDay;
	str << ".";
	str.width(2); str << time.wHour << time.wMinute << time.wSecond << ".";
	str.width(3); str << time.wMilliseconds;
	return str.str();
}

void util::createDirectory(string const& path)
{
	if (CreateDirectoryA(path.c_str(), NULL)) return;

	DWORD error = GetLastError();
	if (error == ERROR_ALREADY_EXISTS) return;

	ostringstream msg;
	msg << "CreateDirectory('" + path + "') failed with error " << error;
	throw runtime_error(msg.str());
}

bool util::fileExists(string const& path)
{
	DWORD dwAttr = GetFileAttributesA(path.c_str());
	if (dwAttr == INVALID_FILE_ATTRIBUTES) return false;
	return (dwAttr & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

HANDLE util::duplicateHandle(HANDLE h)
{
	static HANDLE hProcess = GetCurrentProcess();
	HANDLE result;
	if (!DuplicateHandle(hProcess, h, hProcess, &result, 0, TRUE, DUPLICATE_SAME_ACCESS)) return NULL;
	return result;
}