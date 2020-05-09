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
  A complete list of functions is given below.
  If function argument error occurs, the parser will report error.
- An "eneity addition" statement adding an entity (e.g. bars, labels) to be plotted or will help plotting.
  The entity addition statements are of the form "+\[entity name\] arg1 arg2 ...".
  Argument rules are identical to those of function calls.

All statements are terminated with a semicolon at the end (comments can still go after that), and must complete
within a single line.

### Lists of Properties

| Name | Description | Type  |
|:----:|:--------|:-----:|
| xtitle | X axis title | string |
| ytitle | Y axis title | string |
| fig_filename | The file name when saving a figure | string |
| legend_filename | The file name when saving only the legend | string |
| width | The width of the figure | float\[0.0, +inf\] |
| height | The height of the figure | float\[0.0, +inf\] |
| legend_rows | Number of rows in the legend. This applies to both figure and legend plot | int\[1, +inf\] |
| legend_font_size | Font size in the legend. This applies to both figure and legend plot | int\[1, +inf\] |
| legend_pos | Position of the legend. Only applies to figure. Refer to matplotlib doc for valid values | string |
| xtick_font_size | Font size for X axis ticks | int\[1, +inf\] |
| xtick_rotation | Rotation for X axis ticks | int\[0, 359\] |
| ytick_font_size | Font size for Y axis ticks | int\[1, +inf\] |
| ytick_rotation | Rotation for Y axis ticks | int\[0, 359\] |
| xtitle_font_size | Font size for X axis title | int\[1, +inf\] |
| ytitle_font_size | Font size for Y axis title | int\[1, +inf\] |
| bar_text_font_size | Font size for bar texts | int\[1, +inf\] |
| bar_text_rotation | Rotation for bar texts | int\[0, 359\] |
| bar_text_decimals | Number of digits to print for bar texts | int\[-inf, +inf\] |
| bar_text_rtrim | Whether to trim trailing zeros after the decimal point | int\[0, 1\] |
| xlim_left | X axis left limit | float\[-inf, +inf\] |
| xlim_right | X axis right limit | float\[-inf, +inf\] |
| ylim_top | Y axis top limit | float\[-inf, +inf\] |
| ylim_bottom | Y axis bottom limit | float\[-inf, +inf\] |
| dry_run | Whether to execute the script. 0 means true. 1 means false. 2 means only show the plot on-screen. | \[0, 2\] or "disabled", "enabled", "show" |

### Lists of Functions

| Name | Description | Arguments |
|:----:|:------------|:----------|
| print | Print internal data structures or a formatting string using property name | If followed by param, plot, version, color or hatch, print the corresponding data structure. Optional argument is verbose, which enabled verbose printing. If followed by a format string, then print format string. Arguments are given after the format string. Type error will occur if argument types do not match format string's specifier.|
| reset | Reset an internal data structure | Follwed by param, buf or plot, which resets the corresponding internal data structure to the initial state as if they were just initialized (e.g. for buffer we clear all contents, and copy over the standard preamble). |
|  |  |  |
|  |  |  |
|  |  |  |
|  |  |  |
|  |  |  |
|  |  |  |

## Decimals