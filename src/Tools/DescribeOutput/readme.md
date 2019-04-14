# DescribeOutput

Writes description of the console in which it is executing (or lack thereof) to standard output. Useful for detecting properties of hidden consoles.

## Usage
`DescribeOutput [-p]`
### Options
`-p`: Wait for a key press  before exiting. This will hang if output console is not visible.

## Sample output
```
The process has a console window
Output handle 12 has type 2, character device
Output console window is 118x40, buffer is 118x40
Console code page is 866
```

Writes this to the standard output and to a file named `DescribeOutput.txt`.