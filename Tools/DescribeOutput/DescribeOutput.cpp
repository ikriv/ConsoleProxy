// DescribeOutput.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <sstream>
#include <iostream>
#include <fstream>

using namespace std;

FILE* logFile;

void message(char const* text)
{
	fputs(text, logFile);
	fputs("\r\n", logFile);
	puts(text);
}

char const * GetHandleTypeText(DWORD type)
{
	switch (type)
	{
	case FILE_TYPE_CHAR: return "character device"; 
	case FILE_TYPE_DISK: return "file"; 
	case FILE_TYPE_PIPE: return "pipe"; 
	default: return "unknown";
	}
}

int main()
{
	ostringstream msg;

	HWND hConsole = GetConsoleWindow();
	msg << (hConsole ? "The process has a console window" : "There is no console window") << endl; 

	HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	msg << "Output handle " << (int)hOutput;

	DWORD type = GetFileType(hOutput);
	msg << " has type " << type << ", " << GetHandleTypeText(type) << endl;

	if (type == FILE_TYPE_CHAR)
	{
		_CONSOLE_SCREEN_BUFFER_INFO info;
		if (GetConsoleScreenBufferInfo(hOutput, &info))
		{
			msg << "Output console window is " 
				<< info.srWindow.Right - info.srWindow.Left + 1 << "x" << info.srWindow.Bottom - info.srWindow.Top + 1 
				<< ", buffer is " << info.dwSize.X << "x" << info.dwSize.Y << endl;
		}
		else
		{
			msg << "Consoel buffer info not available" << endl;
		}
	}

	msg << "Console code page is " << GetConsoleOutputCP() << endl;

	cout << msg.str();

	ofstream log;
	log.open("DescribeOutput.txt", ios::out | ios::trunc);

	log << msg.str();

    return 0;
}

