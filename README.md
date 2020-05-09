# matplotlib-c
C language wrapper of the Python drawing library matplotlib

## Script

### Primitives

There are three types of primitives in the script:

- An integer in C encoding. Only decimals are supported (octal and hexadecimal are not supported, 
  and it is unlikely you will need them).
  Although the parser happily accepts full sized 64 bit signed integers, in most cases they will
  be internally converted to 32 bit integers, losing precision if the value is too large to be represented by a 
  32 bit integer.
  This should be a rare case, if at all.
- A floating pointer number in C encoding. We support both decimal-point and scientific forms.
  We only perform loose checks on floating point numbers, and users should be aware not to abuse the parser, which
  may cause undefined bahavior.
- A string in C encoding. Escaped characters are prefixed with '\\' just in C language. We only support a subset
  of escaped characters, i.e. \\n, \\v, \\r, \\\\, \\" and \\'. Strings could span lines without panicing the parser.
  Adjacent strings, however, are not joined automatically as in C preprocessor, since this will cause issues for 
  argument passing.
- An identifier in C encoding. We fully comply with C language rules.

### Statements

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

### Lists of Properties

| Name | Meaning |
|:----:|:--------|
|  | |