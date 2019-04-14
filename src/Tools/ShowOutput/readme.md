# ShowOutput
 A GUI application that launches the child process, redirects its output to a pipe, converts result to UNICODE and displays it in a message box. Uses system OEM encoding for everything except detached children. Uses ANSI encoding for DETACHED_PROSESS children. 
 
 ## Usage
 `ShowOutput [-c|-d|-w] command
 
 ## Options
 
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
