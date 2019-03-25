#pragma once

#include <string>
typedef void *HANDLE;

// Utility functions that call Win32 API
class util
{
public:
	// gets system environment variable by name
	static std::string getEnvVar(std::string const& name);

	// Given "c:\\foo\\bar\\baz.exe" returns "baz"
	static std::string getFileNameWithoutExtension(std::string const& path);

	// Returns current UTC time in the following format: yyyyMMdd.HHmmss.fff. 
	// For example March 20, 2019 19:45:16.987 UTC becomes "20190320.194516.987"
	static std::string getFormattedTimeUtc();

	// Creates a directory using Win32 API. It can only can one level of new directories.
	static void createDirectory(std::string const& path);

	// Returns true if file exists, false otherwise
	static bool fileExists(std::string const& path);

	// Duplicates a handle
	static HANDLE duplicateHandle(HANDLE h);

};