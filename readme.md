# Console Proxy Project
See also: http://ikriv.com/dev/cpp/ConsoleProxy

`ConsoleProxy` project is a set of tools used to research behavior of console output in Windows, in particular
 * Implications of process creation flags like DETACHED_PROCESS
 * Launching console from a GUI application and redirecting its output
 * Under what cricumstances new consoles are created
 * International characters encoding, code pages, etc.
 * Visible and invisible console console buffers

## ConsoleProxy Tool

ConsoleProxy is a sample logging proxy that can transparently replace any console application and log its usage to a file.

To proxy an application named `foo.exe` do the following:

 1. Rename `ConsoleProxy.exe` to `foo.exe`.
 1. If necessary, remove the original `foo.exe` from PATH and add path to the proxied version instead.
 1. Create environment variable `NATIVE_foo` and set its value to the location of the original foo.exe.
 
When invoked under name `foo.exe`, ConsoleProxy creates a log file under `%localappdata%\ConsoleProxy\Foo` folder, locates the original `foo.exe` via `NATIVE_foo` environment variable, and launches it with the same arguments as itself.

## ConsoleHelloWorld
A .NET console application that prints "Hello World!" in English and in Russian to standard output. Useful for debugging issues with international charactersa and code pages. 

Output:

```
Hello world!
Здравствуй, мир!
```
## DescribeOutput

Writes description of the console in which it is executing (or lack thereof) to standard output. Useful for detecting properties of hidden consoles.

Sampel output:
```
The process has a console window
Output handle 12 has type 2, character device
Output console window is 118x40, buffer is 118x40
Console code page is 866
```

Writes this to the standard output and to a file named `DescribeOutput.txt`.

## Exec
A tool that executes given command line with various process creation flags. 

Options:
  `-a`: Set output code page to ANSI, i.e. return value of `GetACP()` before launching the child
    `-c`: Use CREATE_NEW_CONSOLE flag
  `-d`: Use DETACHED_PROCESS flag
  `-r`: Explicitly redirect child's standard input and output to Exec's own
  `-w`: Use CREATE_NO_WINDOW flag
  `-0`: Use default flags (0) when creating the child process. 
 
 All options *must* be provided after one dash sign. Multiple arguments starting with dash won't be processed. Exec can be used recursively, e.g. 
 `Exec -acr Exec -d DescribeOutput`
 
 ## ShowOutput
 A GUI application that launches the child process, redirects its output to a pipe, converts result to UNICODE and displays it in a message box. Uses system OEM encoding for everything except detached children. Uses ANSI encoding for DETACHED_PROSESS children. Accepts options siimialr to `Exec`:
    `-c`: Use CREATE_NEW_CONSOLE flag
  `-d`: Use DETACHED_PROCESS flag
  `-w`: Use CREATE_NO_WINDOW flag

No option means use zero (0) for flags. 

Examples:

```
ShowOutput DescribeOutput
ShowOutput -d ConsoleHelloWorld
ShowOutput -d Exec -cr ConsoleHelloWorld
ShowOutput Exec -dr DescribeOutput
```
  