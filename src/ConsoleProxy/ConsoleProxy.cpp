#include <Windows.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "..\Common\ArgsUtil.h"
#include "Util.h"

using namespace std;

// Returns path of the executable to run based on the environment variable NATIVE_{programname}
string getProgToRun(string const& progName)
{
	string envVarName = "NATIVE_" + progName;
	string path = util::getEnvVar(envVarName);
	if (!path.size()) throw runtime_error("Environment variable not found: " + envVarName);
	return path;
}

// Runs given console program and writes command arguments and exit code to the log
int runProxy(int argc, char** argv, string const& progName, ostream& log)
{
	log << GetCommandLineA() << endl;

	auto progToRun = getProgToRun(progName);
	if (!util::fileExists(progToRun)) throw runtime_error("File does not exist: " + progToRun);

	DWORD flags = 0;
	bool forceRedirect = false;

	if (!GetConsoleWindow() && !GetConsoleOutputCP())
	{
		// Our process does not have console window, and output codepage is zero => we are detached.
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
		// Explicitly redirect child process' input and output to our own
		si.dwFlags = STARTF_USESTDHANDLES;
		si.hStdInput = util::duplicateHandle(GetStdHandle(STD_INPUT_HANDLE));
		si.hStdOutput = util::duplicateHandle(GetStdHandle(STD_OUTPUT_HANDLE));
		si.hStdError = util::duplicateHandle(GetStdHandle(STD_ERROR_HANDLE));
	}

	// Build command line from the child process name and our arguments starting from argv[1], escaped using ArgsUtil
	int childArgc = argc - 1;
	char** childArgv = argv + 1;
	string args = ArgsUtil<char>::EscapeArgs(childArgc, childArgv);
	string command = args.size() ? progToRun + " " + args : progToRun;

	log << "Executing [" << command << "]" << endl;

	// Start the child process
	PROCESS_INFORMATION pi;

	if (!CreateProcess(NULL, &command.front(), NULL, NULL, TRUE, flags, NULL, NULL, &si, &pi))
	{
		DWORD error = GetLastError();
		ostringstream msg;
		msg << "CreateProcess() failed with error " << error;
		throw runtime_error(msg.str());
	}

	CloseHandle(pi.hThread);

	// Wait for the child process to complete and record the exit code in the log
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

// Creates log directory %LocalAppData%\ConsoleProxy\{progName}
string createLogDir(string const& progName)
{
	string localAppData = util::getEnvVar("LocalAppData");
	if (!localAppData.size()) throw runtime_error("Environment variable LocalAppData not found");

	string subDir = localAppData + "\\ConsoleProxy";
	util::createDirectory(subDir);

	string logDir = subDir + "\\" + progName;
	util::createDirectory(logDir);

	return logDir;
}

// Gets (hopefuly unique) log file name based on curent UTC time and process ID
string getLogFileName(string const& progName)
{
	ostringstream str;
	str << progName << "." << util::getFormattedTimeUtc() << ".p" << GetCurrentProcessId() << ".log";
	return str.str();
}

// Creates log output stream
unique_ptr<ofstream> createLog(string const& progName)
{
	string logDir = createLogDir(progName);
	string logPath = logDir + "\\" + getLogFileName(progName);
	return unique_ptr<ofstream>(new ofstream(logPath));
}

// Runs the proxy by creating the log and calling runProxy(). Log creation exceptions are not handled
int mainImpl(int argc, char** argv)
{
	auto progName = util::getFileNameWithoutExtension(argv[0]);
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

// Runs the proxy by calling mainImpl() and handling exceptions
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