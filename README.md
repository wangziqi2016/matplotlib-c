# matplotlib-c
C language wrapper of the Python drawing library matplotlib

## Script

The plotting script is a text file consisting of three types of statements:

- A "property assignment" statement changing the environmental variables or plotting parameters. 
  These statements appear as assignments of the form ".\[prop_name\] = \[value\]", in which the name 
  must be defined, and value must be of the appropriate type.
  A complete list of properties is given below.
- A "function call" statement invoking a pre-defined procedure with optional arguments. 
  Function calls are of the form "!\[function name\] arg1 arg2 ...".
  Some arguments are optional, while some others can be of various types.
  If function argument error occurs, the parser will report error.
- An "eneity addition" statement adding an entity (e.g. bars, labels) to be plotted or will help plotting.
  The entity addition statements are of the form "+\[entity name\] arg1 arg2 ...".
  Argument rules are identical to those of function calls.

All statements are terminated with a semicolon at the end (comments can still go after that), and must complete
within a single line.
