# matplotlib-c
C language wrapper of the Python drawing library matplotlib

## Script

### Overview

The script is not intended to be used as a programming language, not even a simple one. No notion of variables or 
control flows are supported. The interpreter simply begins from the file start, runs a state machine, and interprets the 
script token-by-token. We do have a few simple rules for how tokens are constructed, but still, no data structure is 
supported.

You would expect the plotting script to be imperative: Instead of detailing every single step to generate the plot, you
simply express the basic structure and parameters using one of the three types of statements. The plotting 
backend will then compute coordinates and plotting arguments, generate the Python source code compatible with matplotlib, 
and call Python interpreter to eventually materialize the drawing. Users only need to memorize a small subset of concepts 
compared with what is in matplotlib. In addition, low-level concepts such as bars, texts and legends are highly abstracted
to make common tasks easier and less effort-taking. Users may still want to dump the final script into a file, if further 
tweaks are needed.

### Primitives

There are several types of primitives in the script:

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
  
There are also several derived but commonly used types:

- A file name indicator, beginning with '@' follwed by a string primitive, indicates that the string should be used as a file name.
  Note that for saving files this is not needed.
- Color literal, which is a string of form "#RRGGBB". Spaces will not be ignored, and will incur parsing error.
- Hatch literal, which is a string of form "ch" where ch is the hatch character. Only single character hatch is supported
  (although matplotlib may support multi-char hatches).

### Statements

The plotting script is a text file consisting of three types of statements:

- A "property assignment" statement changing the environmental variables or plotting parameters. 
  These statements appear as assignments of the form ".\[prop_name\] = \[value\]", in which the name 
  must be defined, and value must be of the appropriate type.
  A complete list of properties is given below.
- A "function call" statement invoking a pre-defined procedure with optional arguments. 
  Function calls are of the form "!\[function name\] arg1 arg2 ...".
  Some arguments are optional, while some others can be of various types.
  All functions can take symbol "?" (not a string) as its first argument to print the help string.
  A complete list of functions is given below.
  If function argument error occurs, the parser will report error.
- An "entity addition" statement adding an entity (e.g. bars, labels) to be plotted or will help plotting.
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
| info | Whether to notify users of internal state change | \[0, 1\] or "disabled", "enabled" |

### Lists of Functions

| Name | Description | Arguments |
|:----:|:------------|:----------|
| print | Print internal data structures or a formatting string using property name | If followed by param, plot, version, color, hatch, etc, print the corresponding data structure (please refer to the help string for complete list). If followed by a format string, then print format string. Arguments are given after the format string. Type error will occur if argument types do not match format string's specifier. |
| reset | Reset an internal data structure | Follwed by param, buf or plot, which resets the corresponding internal data structure to the initial state as if they were just initialized (e.g. for buffer we clear all contents, and copy over the standard preamble). |
| save_fig | Save the current plot to an external file | Optional file name can be given as a string, which overrides existing figure file name if there is one. If not given, then use the existing file name. If neither is present, report error. |
| save_legend | Save legend using current bar types in the plot | Same rule for file name, except that we use legend file name stored in the plot object. |
| create_fig | Initialize width and height for the current figure | Two optional arguments, width and height. If not specified then just use default values. This function can only be called once before a re-initialization or reset. |
| set_color_scheme | Sets color scheme used to create bar types and labels | If followed by a string, the string is used as the scheme name, and the built-in database is searched for that name. Report error if name does not exist. If followed by a file indicator ('@' and
a string), the file will be used as import file to load a color scheme. The file consists of one color code per line (spaces and empty lines will be ignored). An optional argument can also be given to indicate the begin offset in the color scheme. The begin offset defaults to zero, and is used as the current position for initializing bar types. |
| set_hatch_scheme | Sets hatch scheme used to create bar types and labels | Same as set_color_scheme, except that the hatch scheme file consists of one hatch (single character) per line. |
| test_hatch | Output a graph with all current hatchs | The save file name is given as argument. Offset is ignored, and the test graph always start from element zero. |
| test_color | Output a graph with all current colors | The save file name is given as argument. Offset is ignored, and the test graph always start from element zero. |

### List of Entities

| Name | Explanation | Arguments |
|:----:|:------------|:----------|
| bar_type | An object containing bar label, color and hatch. It defines the appearance of a bar and entries in the legend | First mandatory argument is type label, which is of string type and must not collide with an existing type. The second and third arguments are color and hatch, both being optional. If user does not give color and/or hatch, the one from color scheme and/or hatch scheme will be used, and the corresponding offset will be incremented. Overflow will be reported as errors. Empty string is handled as if the argument were not provided. They can be used to specify the hatch without explicitly specifying the color. |

## Decimals

Users could control the number of digits after the decimal point and/or the number of significant digits before the 
decimal point for the text printed over bars. Property "bar_text_decimals" specifies how many digits we preserve after 
the decimal point, if it is positive value, or how many digits will be ignored before the decimal point, if it is 
a negative value. If the number of digits exceeds the actual number of digits, zero will be padded, only if the value
is positive.

For example:

- 1234.56789, decimals = 3, output 1234.568 (rounded upwards)
- 1234.56789, decimals = 6, output 1234.567890 (decimals exceeding number of digits after the point)
- 1234.56789, decimals = -2, output 1200 (negative decimals)
- 1234.56789, decimals = -10, output 0 (decimals exceeding number of digits before the point)
- 56789.1234, decimals = -2, output 56800 (rounded upwards)

Users could further control whether trailing zeros of a floating point after the decimal point should be chopped off
to reduce the length of the text. This is enabled by setting "bar_text_rtrim" to 1.

## Adding new Members to `plot_param_t`

First, you need to add the field to plot_param_t, and then assign a default value in default_param. 
Then, enum definitions for the new field should be added to the anonymous enum in the header.
Next, add search table entry to `parse_cb_top_props`, such that functions setting and reading this 
value could map the string key to the enum constant.
The next step to handle the constants in two switch statements, one in `parse_top_property`, 
the other in `parse_print_prop`.
Eventually, the field should also be printed by `plot_param_print`.
