# Exec/ExecW
Executes given command with various process creation flags. `Exec` is a console application, `ExecW` is a Windows appliaction without a console. Otherwise, they behave the same.

## Usage

`Exec[W] [options] command`

Options are specified as a single argument starting with "dash", e.g. `-ac`. The form `-a -c` is *not supported*. 

Examples:
`Exec -ac DescribeOutput -p`
`Exec -acr Exec -d DescribeOutput`

## Options

  `-a`: Set console output code page to ANSI, i.e. to the return value of `GetACP()` before launching the child. This has no effect for `ExecW`, since it does not have a console whose code page can be set.
  
  `-c`: Use CREATE_NEW_CONSOLE flag
  
  `-d`: Use DETACHED_PROCESS flag
  
  '-p': `Exec`: wait for a key press before exit. `ExecW`: show a message box before exit.
  
  `-r`: Explicitly redirect child's standard input and output to the Exec's own
  
  `-w`: Use CREATE_NO_WINDOW flag
  
  `-0`: Use no flags (0) when creating the child process. This is the default.
 
 All options *must* be provided after one dash sign. Multiple arguments starting with dash won't be processed. Exec can be used recursively, e.g. 
 `Exec -acr Exec -d DescribeOutput`