#pragma once

#include "CommandLine.h"
#include "..\..\Exec\ArgsUtil.h"

class ShowOutputArgs
{
private:
	std::wstring _processToRun;
	DWORD _flags;

public:
	ShowOutputArgs() : _flags(0) {}

	bool Parse(CommandLine const& commandLine)
	{
		int argc = commandLine.argc();
		if (argc == 0) return false;
		
		bool hasOption = true;
		
		std::wstring option = commandLine.argv(0);
		if (option == L"-d" || option == L"--detached")
		{
			_flags = DETACHED_PROCESS;
		}
		else if (option == L"-w" || option == L"--no-window")
		{
			_flags = CREATE_NO_WINDOW;
		}
		else if (option == L"-c" || option == L"--new-console")
		{
			_flags = CREATE_NEW_CONSOLE;
		}
		else
		{
			// unknown option or first argument is not an option
			hasOption = false;
		}

		int skipArgs = hasOption ? 1 : 0;
		if (argc <= skipArgs)
		{
			// nothing follows the options, it is not right
			return false;
		}

		_processToRun = ArgsUtil<wchar_t>::EscapeArgs(commandLine.argc() - skipArgs, commandLine.argv() + skipArgs);
		return true;
	}

	static LPCWSTR usageInfo()
	{
		return L"Usage: ShowOutput [options] what_to_run\r\n"
			L"Options:\r\n"
			L"\t-c --new-console: use CREATE_NEW_CONSOLE\r\n"
			L"\t-d, --detached: use DETACHED_PROCESS\r\n"
			L"\t-w --no-window: use CREATE_NO_WINDOW\r\n";
	}

	std::wstring const& processToRun() const { return _processToRun; }
	DWORD flags() const { return _flags; }
};
