#pragma once

#include "CommandLine.h"

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
		if (argc == 0 || argc > 2) return false;
		
		if (argc == 1)
		{
			_processToRun = commandLine.argv(0);
			return true;
		}

		// 2 arguments
		std::wstring option = commandLine.argv(0);
		if (option == L"-d" || option == L"--detached")
		{
			_flags = DETACHED_PROCESS;
		}
		else if (option == L"-n" || option == L"--no-window")
		{
			_flags = CREATE_NO_WINDOW;
		}
		else
		{
			// unknown option
			return false;
		}

		_processToRun = commandLine.argv(1);
		return true;
	}

	std::wstring const& processToRun() const { return _processToRun; }
	DWORD flags() const { return _flags; }
};
