#include <Windows.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "..\Common\ArgsUtil.h"

using namespace std;

string getEnvVar(string const& name)
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

string getFileNameWithoutExtension(string const& path)
{
	int start = path.find_last_of('\\');
	if (start == string::npos) start = 0; else ++start;
	if (start >= (int)path.size()) return string();

	int end = path.find_last_of('.');
	if (end == string::npos || end <= start) end = path.size();

	return path.substr(start, end - start);
}

string getFormattedTimeUtc()
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

string getLogFileName(string const& progName)
{
	ostringstream str;
	str << progName << "." << getFormattedTimeUtc() << ".p" << GetCurrentProcessId() << ".log";
	return str.str();
}

void createDirectory(string const& path)
{
	if (CreateDirectoryA(path.c_str(), NULL)) return;

	DWORD error = GetLastError();
	if (error == ERROR_ALREADY_EXISTS) return;

	ostringstream msg;
	msg << "CreateDirectory('" + path + "') failed with error " << error;
	throw runtime_error(msg.str());
}

bool fileExists(string const& path)
{
	DWORD dwAttr = GetFileAttributesA(path.c_str());
	if (dwAttr == INVALID_FILE_ATTRIBUTES) return false;
	return (dwAttr & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

string createLogDir(string const& progName)
{
	string localAppData = getEnvVar("LocalAppData");
	if (!localAppData.size()) throw runtime_error("Environment variable LocalAppData not found");

	string subDir = localAppData + "\\ConsoleProxy";
	createDirectory(subDir);

	string logDir = subDir + "\\" + progName;
	createDirectory(logDir);

	return logDir;
}

HANDLE duplicateHandle(HANDLE h)
{
	static HANDLE hProcess = GetCurrentProcess();
	HANDLE result;
	if (!DuplicateHandle(hProcess, h, hProcess, &result, 0, TRUE, DUPLICATE_SAME_ACCESS)) return NULL;
	return result;
}

string getProgToRun(string const& progName)
{
	string envVarName = "NATIVE_" + progName;
	string path = getEnvVar(envVarName);
	if (!path.size()) throw runtime_error("Environment variable not found: " + envVarName);
	return path;
}

int runProxy(int argc, char** argv, string const& progName, ostream& log)
{
	log << GetCommandLineA() << endl;

	auto progToRun = getProgToRun(progName);
	if (!fileExists(progToRun)) throw runtime_error("File does not exist: " + progToRun);

	DWORD flags = 0;
	bool forceRedirect = false;

	if (!GetConsoleWindow() && !GetConsoleOutputCP())
	{
		// We are detached :(
		// Start child process with CREATE_NO_WINDOW flag.
		// Plus: this automatically redirects output of grandchildren
		// Con: this sets wrong code page for international output.
		flags = CREATE_NO_WINDOW;
		forceRedirect = true;
	}

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	if (forceRedirect)
	{
		si.dwFlags = STARTF_USESTDHANDLES;
		si.hStdInput = duplicateHandle(GetStdHandle(STD_INPUT_HANDLE));
		si.hStdOutput = duplicateHandle(GetStdHandle(STD_OUTPUT_HANDLE));
		si.hStdError = duplicateHandle(GetStdHandle(STD_ERROR_HANDLE));
	}

	int childArgc = argc - 1;
	char** childArgv = argv + 1;
	string args = ArgsUtil<char>::EscapeArgs(childArgc, childArgv);
	string command = args.size() ? progToRun + " " + args : progToRun;

	log << "Executing [" << command << "]" << endl;

	PROCESS_INFORMATION pi;

	if (!CreateProcess(NULL, &command.front(), NULL, NULL, TRUE, flags, NULL, NULL, &si, &pi))
	{
		DWORD error = GetLastError();
		ostringstream msg;
		msg << "CreateProcess() failed with error " << error;
		throw runtime_error(msg.str());
	}

	CloseHandle(pi.hThread);
	WaitForSingleObject(pi.hProcess, INFINITE);

	DWORD code;
	if (!GetExitCodeProcess(pi.hProcess, &code))
	{
		DWORD error = GetLastError();
		log << "Warning: GetExitCodeProcess() failed with error " << error << endl;
		return -1;
	}

	log << "Exit code " << code << endl;
	CloseHandle(pi.hProcess);

	return (int)code;
}

unique_ptr<ofstream> createLog(string const& progName)
{
	string logDir = createLogDir(progName);
	string logPath = logDir + "\\" + getLogFileName(progName);
	return unique_ptr<ofstream>(new ofstream(logPath));
}

int mainImpl(int argc, char** argv)
{
	auto progName = getFileNameWithoutExtension(argv[0]);
	unique_ptr<ofstream> pLog = createLog(progName);

	try
	{
		return runProxy(argc, argv, progName, *pLog);
	}
	catch (exception const& err)
	{
		*pLog << "Error: " << err.what() << endl;
		throw;
	}
}

int main(int argc, char** argv)
{
	try
	{
		return mainImpl(argc, argv);
	}
	catch (exception const& err)
	{
		cerr << err.what() << endl;
	}
}