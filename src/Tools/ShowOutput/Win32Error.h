#pragma once

class Win32Error
{
	std::wstring _message;

public:
	explicit Win32Error(std::wstring const& method)
	{
		DWORD error = GetLastError();
		std::wostringstream msg;
		msg << method << " failed. Error " << error;
		_message = msg.str();
	}

	std::wstring const& message() const { return _message; }
};
