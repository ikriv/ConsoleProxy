# Console Proxy Project

This project implements the `ConsoleProxy` tool that acts as a "logging decorator" for console applications. When called, it logs its command line arguments in a file and calls the original application.

See also: https://ikriv.com/dev/cpp/ConsoleProxy


## ConsoleProxy Tool

ConsoleProxy is a sample logging proxy that can transparently replace any console application and log its usage to a file.

### Usage
To proxy an application named `foo.exe` do the following:
 1. Rename `ConsoleProxy.exe` to `foo.exe`.
 1. If necessary, remove the original `foo.exe` from PATH and add path to the proxied version instead.
 1. Create environment variable `NATIVE_foo` and set its value to the location of the original foo.exe.
 
When invoked under name `foo.exe`, ConsoleProxy creates a log file under `%localappdata%\ConsoleProxy\Foo` folder, locates the original `foo.exe` via `NATIVE_foo` environment variable, and launches it with the same arguments as itself.

## Additional exploratory tools:

|Project                                               | Description                                                                                  |
|------------------------------------------------------|----------------------------------------------------------------------------------------------|
|[ConsoleHelloWorld](src/Tools/ConsoleHelloWorld/readme.md)|Prints "Hello, World" in English and Russian to test international characters encoding issues.|
|[DescribeOutput](src/Tools/DescribeOutput/readme.md)      |Describes current console window and prints information to standard output and to a file.     |
|[Exec](src/Tools/Exec/readme.md)                          |Console application that executes a command with given process creation flags.                |
|[ExecW](src/Tools\Exec/readme.md)                         |GUI version of `Exec`.                                                                        |
|[ShowOutput](src/Tools/ShowOutput/readme.md)              |GUI application that executes a command with given process creation flags, redirects output to a pipe and displays it in a message box.|

## Research Subjects

 * [Exact meaning of console process creation flags like DETACHED_PROCESS](https://ikriv.com/dev/cpp/ConsoleProxy/flags):
   * Under what cricumstances new consoles are created
   * Where does the standard output go
   * Visible and invisible console console buffers
 * [International characters encoding, code pages, etc.](https://ikriv.com/dev/cpp/ConsoleProxy/codepages)
 * [How to properly escape command line arguments](https://ikriv.com/dev/cpp/ConsoleProxy/cmdline).
