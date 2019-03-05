#include "pch.h"
#include "..\..\Exec\ArgsUtil.h"

using namespace std;
using namespace testing;

class ArgList
{
	string _expectedResult;
	vector<string> _args;

public:
	ArgList(string const& expectedResult, initializer_list<string> args)
		:
		_expectedResult(expectedResult),
		_args(args)
	{
	}

	ArgList(ArgList const& other)
	{
		_expectedResult = other._expectedResult;
		_args = other._args;
	}

	int argc() const { return _args.size(); }
	vector<string> const& args() const { return _args; }
	vector<string>::const_iterator argv() const { return _args.begin(); }
	string const& expectedResult() const { return _expectedResult; }

	// Create a free inline friend function.
	friend std::ostream& operator<<(std::ostream& os, ArgList const& list) 
	{
		bool isFirst = true;
		os << "{";
		for (auto arg : list.args())
		{
			if (!isFirst) os << ",";
			isFirst = false;
			os << arg;
		}

		return os << "}";
	}
};

class ArgsUtilTests : public TestWithParam<ArgList> {};

TEST_P(ArgsUtilTests, TestEscapeArgs)
{
	auto p = GetParam();
	string escapedCommandLine = ArgsUtil<char>::EscapeArgs(p.argc(), p.argv());

	EXPECT_EQ(p.expectedResult(), escapedCommandLine);
}

INSTANTIATE_TEST_CASE_P(ArgsUtilTestCase, ArgsUtilTests, Values(
	ArgList("foo", { "foo" }),
	ArgList("foo bar", { "foo", "bar" }),
	ArgList("\"foo bar\"", { "foo bar" }),
	ArgList("quote\\\"inside", { "quote\"inside" }),
	ArgList("bask\\slash", { "bask\\slash" }),
	ArgList("\"spaces, quote and bask\\\\\"slashes\"", { "spaces, quote and bask\\\"slashes" }))
);
