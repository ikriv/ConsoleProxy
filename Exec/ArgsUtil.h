#pragma once

template<typename tchar>
class ArgsUtil
{
	typedef std::basic_string<tchar> string;
	typedef std::basic_ostringstream<tchar, std::char_traits<tchar>, std::allocator<tchar>> stream;

	static const tchar SPACE = (tchar)' ';
	static const tchar TAB = (tchar)'\t';
	static const tchar QUOTE = (tchar)'"';
	static const tchar BACKSLASH = (tchar)'\\';

	static void AddBackSlashes(string& s, int howMany)
	{
		for (int i = 0; i < howMany; ++i) s.push_back(BACKSLASH);
	}

public:

	static string EscapeArg(string const& arg)
	{
		// If the arg contains neither quotes, spaces, or tabs, it can be used as is
		// If it contains spaces or tabs, it must be surrounded with quotes
		// If it contains a quote, it becomes \"
		// If that quote is preceded by one or more backslashes, each backslash must be doubled
		// See documentation for CommandLineToArgvW function and
		// https://docs.microsoft.com/en-us/previous-versions//17w5ykft(v=vs.85), Parsing C++ Command-Line arguments

		bool hasSpaces = false;
		int nBackSlashes = 0;

		string result;

		for (auto c : arg)
		{
			if (c == BACKSLASH)
			{
				++nBackSlashes;
				continue;
			}

			// this is NOT a backslash
			if (nBackSlashes > 0)
			{
				AddBackSlashes(result, nBackSlashes);
			}
			nBackSlashes = 0;

			if (c == SPACE || c == TAB) hasSpaces = true;

			if (c == QUOTE)
			{
				if (nBackSlashes > 0)
				{
					AddBackSlashes(result, 2 * nBackSlashes);
					nBackSlashes = 0;
				}

				result.push_back(BACKSLASH);
			}

			result.push_back(c);
		}

		// add any final backslashes
		if (nBackSlashes > 0) AddBackSlashes(result, nBackSlashes);

		if (hasSpaces) result = QUOTE + result + QUOTE;

		return result;
	}

	template<typename Iterator>
	static string EscapeArgs(int argc, Iterator argv)
	{
		stream output;
		int i = 0;

		for (auto pArg = argv; i<argc; ++i, ++pArg)
		{
			if (i > 0) output << SPACE;
			output << EscapeArg(*pArg);
		}

		return output.str();
	}
};
