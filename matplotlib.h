
#ifndef _MATPLOTLIB_H
#define _MATPLOTLIB_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <error.h>
#include <stdarg.h>
#include <float.h>  // double max
#include <stddef.h> // offsetof()
#include <math.h>   // INFINITY

#include "Python.h"

// Version strings
#define MAJOR_VERSION "1"
#define MINOR_VERSION "0"

//* error handling

// Error reporting and system call assertion
#define SYSEXPECT(expr) do { if(!(expr)) { perror(__func__); exit(1); } } while(0)
#define error_exit(fmt, ...) do { fprintf(stderr, "%s error: " fmt, __func__, ##__VA_ARGS__); exit(1); } while(0);
#ifndef NDEBUG
#define dbg_printf(fmt, ...) do { fprintf(stderr, fmt, ##__VA_ARGS__); } while(0);
#else
#define dbg_printf(fmt, ...) do {} while(0);
#endif

inline static int streq(const char *a, const char *b) { return strcmp(a, b) == 0; }

//* fp_* - String processing of floating point numbers

// Initial size of the buffer - may be larger
#define FP_BUF_SIZE 32
#define FP_MAX_ITER 4

double fp_power10(int num); // Return power of 10
char *fp_print(double num, int frac_count); // Print the fp number to a buffer
char *fp_rtrim(char *buf); // Remove trailing zeros after the decimal point

//* color_* - Color processing

// Size of the color object
#define COLOR_SIZE        sizeof(uint32_t)
// Size of color string (excl. '\0'): # HH HH HH
#define COLOR_STRLEN      7
// Number of elements we allocate when reading from file
#define COLOR_INIT_FILE_COUNT  1

// Macros for extracting color components
#define COLOR_R(x) ((x >> 16) & 0xFF)
#define COLOR_G(x) ((x >> 8) & 0xFF)
#define COLOR_B(x) ((x >> 0) & 0xFF)
// Composing a color using RGB components
#define COLOR_GEN(r, g, b) (((r & 0xFF) << 16) | ((g & 0xFF) << 8) | ((b & 0xFF) << 0))

typedef struct {
  char *name;         // Name of the scheme
  uint32_t *base;     // Points to the array
  int item_count;     // Number of items in the base array
} color_scheme_t;

// Init or copy-init depending on scheme
color_scheme_t *color_scheme_init(const char *name, uint32_t *base, int item_count);
color_scheme_t *color_scheme_init_file(const char *filename);
void color_scheme_free(color_scheme_t *scheme);

extern uint32_t color_scheme_mixed[];
extern uint32_t color_scheme_red[];
extern uint32_t color_scheme_blue[];
extern uint32_t color_scheme_grey[];

// This macro generates an entry in color_schemes
#define COLOR_SCHEME_GEN(name, base) {name, base, sizeof(base) / COLOR_SIZE}

extern color_scheme_t color_schemes[];

void _color_str(uint32_t color, char *buf, int for_latex); // Returns RGB color code
inline static void color_str(uint32_t color, char *buf) { _color_str(color, buf, 0); }
inline static void color_str_latex(uint32_t color, char *buf) { _color_str(color, buf, 1); }

uint32_t color_decode(const char *s); // Decode string representation of color
color_scheme_t *color_find_scheme(const char *name);  // Using string name to locate the color_scheme_t object
color_scheme_t *color_scheme_dup(color_scheme_t *scheme);

void color_scheme_print(color_scheme_t *scheme, int print_content);

//* hatch_*

#define HATCH_SIZE            1
#define HATCH_INIT_FILE_COUNT 1

typedef struct {
  char *name;
  char *base;
  int item_count;
} hatch_scheme_t;

hatch_scheme_t *hatch_scheme_init(const char *name, char *base, int item_count);
hatch_scheme_t *hatch_scheme_init_file(const char *filename);
void hatch_scheme_free(hatch_scheme_t *scheme);

#define HATCH_SCHEME_GEN(name, base) {name, base, sizeof(base) / HATCH_SIZE}

extern char hatch_scheme_all[];
extern int hatch_scheme_all_count; 
extern char *hatch_scheme_default;

extern hatch_scheme_t hatch_schemes[];

char hatch_decode(const char *s);
int hatch_is_valid(char hatch);
hatch_scheme_t *hatch_find_scheme(const char *name);
hatch_scheme_t *hatch_scheme_dup(hatch_scheme_t *scheme);
void hatch_scheme_print(hatch_scheme_t *scheme, int print_content);

//* py_* - Python interpreter

typedef struct {
  uint64_t padding; // Avoid allocating zero bytes; Some malloc() do not handle this well
} py_t;

py_t *py_init();
void py_free(py_t *py);
void py_run(py_t *py, const char *s);

int py_get_instance_count();

//* vec_t

#define VEC_INIT_COUNT 16

typedef struct {
  void **data;
  int count;
  int capacity;
} vec_t;

vec_t *vec_init();
void vec_free(vec_t *vec);
void vec_append(vec_t *vec, void *p);

inline static void *vec_at(vec_t *vec, int index) { assert(index >= 0 && index < vec->count); return vec->data[index]; }
inline static int vec_count(vec_t *vec) { return vec->count; }

void vec_print(vec_t *vec);

//* buf_* - String buffer

#define BUF_INIT_SIZE 256

typedef struct {
  char *data;
  int capacity; // Actual number of usable bytes
  int size;     // Number of used bytes, including trailing zero
} buf_t;

buf_t *buf_init();          // Default to BUF_INIT_SIZE
buf_t *buf_init_sz(int sz); // Using any init size
void buf_free(buf_t *buf);
void buf_reset(buf_t *buf);

inline static int buf_get_size(buf_t *buf) { return buf->size; }
inline static int buf_get_capacity(buf_t *buf) { return buf->capacity; }
inline static char *buf_c_str(buf_t *buf) { return buf->data; }
inline static int buf_strlen(buf_t *buf) { return buf->size - 1; }

void buf_realloc(buf_t *buf, int target);
void buf_append(buf_t *buf, const char *s);
void buf_putchar(buf_t *buf, char ch);
void buf_concat(buf_t *buf, buf_t *s);
void buf_printf(buf_t *buf, const char *fmt, ...);
void buf_append_color(buf_t *buf, uint32_t color);

void buf_print(buf_t *buf, int content);
void buf_dump(buf_t *buf, const char *filename);

//* bar_t - Bar object

// These objects must be unique, i.e. one object for each label
// Label defines how legend is drawn
typedef struct bar_type_struct_t {
  char *label;       // Has ownership
  char hatch;        // Hatch (filling pattern); '\0' means not present
  uint32_t color;    // RGB color
  int used;          // Whether it has been used in bar() method
} bar_type_t;

bar_type_t *bar_type_init(const char *label);
void bar_type_free(bar_type_t *type);
bar_type_t *bar_type_dup(bar_type_t *type);

void bar_type_print(bar_type_t *type);

typedef struct {
  double height;    // Height of the bar
  double bottom;    // Non-zero means we draw stacked bar
  double pos;       // Offset in the horizontal direction (aligned to the left edge)
  double width;     // Width of the bar
  bar_type_t *type; // Used to draw legend; has no ownership
  char *text;       // Optional text (has ownership if not NULL)
  // TODO: ADD ERROR BAR, WIDTH AND COLOR
  // TODO: LINE WIDTH, LINE PATTERN, LINE COLOR, etc. see 
  // https://matplotlib.org/3.2.1/api/_as_gen/matplotlib.axes.Axes.bar.html
  int inited;       // Whether the bar has been initialized - use this to check error
  int stacked;      // Whether the bar is stacked with the previous one in the group
} bar_t;

bar_t *bar_init();
void bar_free(bar_t *bar);

inline static uint32_t bar_get_color(bar_t *bar) { return bar->type->color; }
inline static char bar_get_hatch(bar_t *bar) { return bar->type->hatch; }
inline static bar_type_t *bar_get_type(bar_t *bar) { return bar->type; }
inline static void bar_set_type(bar_t *bar, bar_type_t *type) { bar->type = type; }

void bar_set_text(bar_t *bar, const char *text);

void bar_print(bar_t *bar);

//* bargrp_t - Bar groups

typedef struct {
  char *name;             // The X tick, if enabled. The group owns this string
  vec_t *bars;            // A vector of bars; Bar group owns the bars
} bargrp_t;

bargrp_t *bargrp_init(const char *name);
void bargrp_free(bargrp_t *grp);
inline static void bargrp_add_bar(bargrp_t *grp, bar_t *bar) { vec_append(grp->bars, bar); }
inline static int bargrp_count(bargrp_t *bargrp) { return vec_count(bargrp->bars); }

void bargrp_print(bargrp_t *grp, int verbose);

//* plot_t - Plotting function

// Maximum size of legend pos (including trailing zero)
#define PLOT_LEGEND_POS_MAX_SIZE  16
#define PLOT_DRY_RUN_DISABLED     0     // Always output
#define PLOT_DRY_RUN_ENABLED      1     // Always not output
#define PLOT_DRY_RUN_SHOW         2     // Not output but show the graph

#define PLOT_DIRECTION_INSIDE  0
#define PLOT_DIRECTION_OUTSIDE 1
#define PLOT_DIRECTION_BOTH    2

#define PLOT_SAVE_MODE_FILE    0        // Save plot to a file
#define PLOT_SAVE_MODE_BUF     1        // Copy plot to a buffer object

struct parse_struct_t;

// This object should not be freed; Always copy it over
typedef struct {
  double width;
  double height;
  // Legend parameters
  int legend_enabled;        // Whether to draw the legend
  int legend_rows;           // By default draw horizontal legend
  int legend_font_size;      // Font size for text in the legend
  char legend_pos[PLOT_LEGEND_POS_MAX_SIZE]; // Legend position; This string should never be freed
  // X Tick
  int xtick_enabled;         // This turns off both labels and tick
  double xtick_length;       // Length of the tick - set to 0.0 to turn off. Set to INFINITY uses default
  int xtick_direction;          // Whether the tick stays inside, outside, or both (0 - inside, 1 - outside, 2 - both)
  int xtick_font_size;
  int xtick_rotation;
  int xtick_label_enabled;   // Whether to draw tick labels
  // Y Tick
  int ytick_enabled;         // Same as above
  double ytick_length;
  int ytick_direction;
  int ytick_font_size;
  int ytick_rotation;
  int ytick_label_enabled;
  // Grid
  int xgrid_enabled;
  int ygrid_enabled;
  // Title
  int xtitle_font_size;
  int ytitle_font_size;
  // Bar text
  int bar_text_font_size;
  int bar_text_rotation;
  int bar_text_decimals;         // Number of decimals after decimal point when printing the bar text
  int bar_text_rtrim;            // Whether trim zeros after the number
  // Hatch and color - These two are set with functions, not properties
  hatch_scheme_t *hatch_scheme;  // Hatch scheme
  int hatch_offset;              // Offset in the array
  color_scheme_t *color_scheme;  // Color scheme
  int color_offset;              // Offset in the array
  // Limits
  double xlim_left;
  double xlim_right;
  double ylim_top;
  double ylim_bottom;
  // Bar group
  double bargrp_space;           // Space between bargrps
  // Debugging
  int dry_run;                   // Whether we actually execute the scripts (will not if set to 1)
  int info;                      // Whether to print info when the internal state changes
} plot_param_t;

void plot_param_copy(plot_param_t *dst, plot_param_t *src);
void plot_param_print(plot_param_t *param, int verbose);

// This stores tick information
typedef struct {
  vec_t *poses;   // Array of 64 bit double (cast to void *)
  vec_t *labels;  // Array of char pointers; Vector has the storage
} plot_tick_t;

plot_tick_t *plot_tick_init();
void plot_tick_free(plot_tick_t *tick);
void plot_tick_append(plot_tick_t *tick, double pos, const char *str);
inline static int plot_tick_count(plot_tick_t *tick) { return vec_count(tick->poses); }
double plot_tick_pos_at(plot_tick_t *tick, int index);
char *plot_tick_label_at(plot_tick_t *tick, int index);

// The following function returns buf_t object contain Python list representation of all elements
buf_t *plot_tick_pos_str(plot_tick_t *tick);
buf_t *plot_tick_label_str(plot_tick_t *tick);

void plot_tick_print(plot_tick_t *tick, int verbose);

extern plot_param_t default_param; // Default param, will be copied over during init
extern const char *plot_preamble; // This is added to the buffer on plot init

typedef struct {
  py_t *py;
  buf_t *buf;
  vec_t *bar_types;                    // Bar types used for legend
  struct parse_struct_t *parse;        // Script parser
  plot_param_t param;                  // Plotting parameters
  // The following are parsed from the script file
  char *xtitle;                        // X title; Specified at top level
  char *ytitle;                        // Y title; Specified at top level
  char *fig_filename;                  // File name to save the figure
  char *legend_filename;               // File name to save legend, if there is one (optional)
  // Bar groups. If not NULL then we are in bar group mode
  bargrp_t *curr_bargrp;
  // List of bar groups we already added
  vec_t *bargrps;
} plot_t;

plot_t *plot_init();
void plot_free(plot_t *plot);

void plot_open(plot_t *plot, const char *filename);
void plot_open_str(plot_t *plot, const char *s);

void plot_add_bar_type(plot_t *plot, const char *label, uint32_t color, char hatch);
bar_type_t *plot_find_bar_type(plot_t *plot, const char *label);

void plot_add_bargrp(plot_t *plot, bargrp_t *grp);
bargrp_t *plot_find_bargrp(plot_t *plot, const char *name);

// This resets all flags that are set during plot_draw()
void plot_reset_flags(plot_t *plot);
void plot_reset_buf(plot_t *plot);

// The following causes scripts to be generated using params and data points
void plot_draw_axis(plot_t *plot);
void plot_draw_bar(plot_t *plot, bar_t *bar); // This draws a single bar given the bar object
void plot_draw_all_bargrps(plot_t *plot); // This computes coordinates and draws each bar individually
void plot_draw_tick(plot_t *plot);
void plot_draw_grid(plot_t *plot);
void plot_draw_limit(plot_t *plot);
void plot_draw_legend(plot_t *plot);
void plot_draw(plot_t *plot); // Generates scripts except save fig

void plot_save_fig(plot_t *plot, const char *filename);

void plot_save_legend_mode(plot_t *plot, int mode, void *arg);
inline static void plot_save_legend_file(plot_t *plot, const char *filename) {
  plot_save_legend_mode(plot, PLOT_SAVE_MODE_FILE, (void *)filename);
}
inline static void plot_save_legend_buf(plot_t *plot, buf_t *buf) {
  plot_save_legend_mode(plot, PLOT_SAVE_MODE_BUF, (void *)buf); // Append the plot into a buffer object
}

void plot_save_color_test_mode(plot_t *plot, int mode, void *arg);
inline static void plot_save_color_test_file(plot_t *plot, const char *filename) {
  plot_save_color_test_mode(plot, PLOT_SAVE_MODE_FILE, (void *)filename);
}
inline static void plot_save_color_test_buf(plot_t *plot, buf_t *buf) {
  plot_save_color_test_mode(plot, PLOT_SAVE_MODE_BUF, (void *)buf);
}

void plot_save_hatch_test_mode(plot_t *plot, int mode, void *arg);
inline static void plot_save_hatch_test_file(plot_t *plot, const char *filename) {
  plot_save_hatch_test_mode(plot, PLOT_SAVE_MODE_FILE, (void *)filename);
}
inline static void plot_save_hatch_test_buf(plot_t *plot, buf_t *buf) {
  plot_save_hatch_test_mode(plot, PLOT_SAVE_MODE_BUF, (void *)buf);
}

// Defines all valid combinations
extern const char *plot_valid_legend_poses[];

void plot_set_legend_rows(plot_t *plot, int rows);
void plot_set_legend_pos(plot_t *plot, const char *pos);

void plot_add_xtick(plot_t *plot, double pos, const char *text);
void plot_add_ytick(plot_t *plot, double pos, const char *text);
void plot_add_x_title(plot_t *plot, const char *title);
void plot_add_y_title(plot_t *plot, const char *title);

void plot_print(plot_t *plot, int verbose);

//* parse_* - String processing

// String argument mapping - used to read arguments that can be passed using both int and str
// The table is terminated with an empty entry with key setting to NULL
typedef struct {
  const char *key;
  int value;
} parse_mapped_arg_entry_t;

#define PARSE_MAPPED_ARG_END {NULL, 0}

extern parse_mapped_arg_entry_t parse_mapped_arg_direction[];

// Generates parse_cb_entry_t entry
#define PARSE_GEN_CB(name, func) {name, {.cb = func}}
#define PARSE_GEN_PROP(name, p) {name, {.prop = p}}

typedef void (*parse_cb_t)(struct parse_struct_t *parse, plot_t *plot);

// plot_param_t fields
enum {
  // plot_t fields
  PARSE_XTITLE,
  PARSE_YTITLE,
  PARSE_FIG_FILENAME,
  PARSE_LEGEND_FILENAME,
  // plot_param_t fields
  PARSE_WIDTH,
  PARSE_HEIGHT,
  // Legend
  PARSE_LEGEND_ENABLED,
  PARSE_LEGEND_ROWS,
  PARSE_LEGEND_FONT_SIZE,
  PARSE_LEGEND_POS,

  PARSE_XTICK_ENABLED,
  PARSE_XTICK_LENGTH,
  PARSE_XTICK_DIRECTION,
  PARSE_XTICK_FONT_SIZE,
  PARSE_XTICK_ROTATION,
  PARSE_XTICK_LABEL_ENABLED,

  PARSE_YTICK_ENABLED,
  PARSE_YTICK_LENGTH,
  PARSE_YTICK_DIRECTION,
  PARSE_YTICK_FONT_SIZE,
  PARSE_YTICK_ROTATION,
  PARSE_YTICK_LABEL_ENABLED,

  PARSE_XGRID_ENABLED,

  PARSE_YGRID_ENABLED,

  PARSE_XTITLE_FONT_SIZE,
  PARSE_YTITLE_FONT_SIZE,

  PARSE_BAR_TEXT_FONT_SIZE,
  PARSE_BAR_TEXT_ROTATION,
  PARSE_BAR_TEXT_DECIMALS,
  PARSE_BAR_TEXT_RTRIM,

  PARSE_XLIM_LEFT,
  PARSE_XLIM_RIGHT,
  PARSE_YLIM_TOP,
  PARSE_YLIM_BOTTOM,

  PARSE_BARGRP_SPACE,

  PARSE_DRY_RUN,     // py_t object
  PARSE_INFO,        // Whether to print information
};

typedef struct {
  const char *name;      // Keyword
  union {
    parse_cb_t cb;       // Call back function
    int prop;            // Pointer to the property
  };
} parse_cb_entry_t;

#define PARSE_INT64_MAX  (0x7FFFFFFFFFFFFFFFL)
#define PARSE_INT64_MIN  (0x8000000000000000L)
#define PARSE_DOUBLE_MAX (DBL_MAX)
#define PARSE_DOUBLE_MIN (DBL_MIN)

typedef struct parse_struct_t {
  char *filename;  // Has ownership; Undefined for string init
  char *s;         // Always point to the start of the string; Read-only; Parser owns the string
  char *curr;      // Current reading location
  int size;        // Size of the string, including '\0'
  int line;        // This is the line we are currently on
  int col;         // This is the char we just read, starting from 1 (0 means no char in the line is read)
} parse_t;

parse_t *_parse_init(char *s);      // This one does not alloc the string
parse_t *parse_init(const char *s); // This one copies the string
parse_t *parse_init_file(const char *filename); // This one reads a file
void parse_free(parse_t *parse);

inline static int parse_get_size(parse_t *parse) { return parse->size; }
inline static int parse_get_line(parse_t *parse) { return parse->line; }
inline static int parse_get_col(parse_t *parse) { return parse->col; }

// Read next char without advancing the read pointer
inline static char parse_peek(parse_t *parse) { return parse->curr[0]; } 
char parse_getchar(parse_t *parse);
void parse_skip_space(parse_t *parse);
char parse_getchar_nospace(parse_t *parse); // Get next char that is not a space (could return '\0')
char parse_peek_nospace(parse_t *parse);

char *parse_copy(parse_t *parse, char *begin, char *end); // Copy the string in the range in malloc'ed memory
char *parse_get_ident(parse_t *parse);   // Get a C-style ident token from the stream
char *parse_get_str(parse_t *parse); // Get a string delimited by a pair of double quotation marks
uint32_t parse_get_color(parse_t *parse); // Parse a string as color code
char *parse_until(parse_t *parse, char ch); // Reads until a certain char is met; Trim left and right before return
double parse_get_double(parse_t *parse); // Reads a double from the stream
double parse_get_double_range(parse_t *parse, double lower, double upper); // Reads a double from the stream
int64_t parse_get_int64(parse_t *parse); // Reads a long int from the stream
int64_t parse_get_int64_range(parse_t *parse, int64_t lower, int64_t upper); // Adds range check
char *parse_get_filename(parse_t *parse); // Returns a file name (prefixed with "@")
void parse_expect_char(parse_t *parse, char ch); // Fetch a char and discard; Report error if mismatch
void parse_expect_char_opt(parse_t *parse, char ch); // Read an optional char and discard; No error if not found

#define PARSE_ARG_NONE   0
#define PARSE_ARG_STR    1
#define PARSE_ARG_NUM    2
#define PARSE_ARG_IDENT  3
#define PARSE_ARG_FILE   4
#define PARSE_ARG_QMARK  5 // This is specifically used to print help string

int parse_next_arg(parse_t *parse); // Argument handling
int parse_get_mapped_arg(parse_t *parse, parse_mapped_arg_entry_t *table); // Argument mapping from str to int

void parse_sort_cb(parse_t *parse, parse_cb_entry_t *table, int count);
parse_cb_entry_t parse_find_cb_entry(parse_t *parse, parse_cb_entry_t *table, int count, const char *name);

void parse_top(parse_t *parse, plot_t *plot); // Parse a script and call plot functions to complete the graph

// Top entity lookup table
extern parse_cb_entry_t parse_cb_top_entities[];
extern const int parse_cb_top_entities_count;

// Top entity parse functions
void parse_top_entity(parse_t *parse, plot_t *plot);
void parse_cb_bar_type(parse_t *parse, plot_t *plot);
void parse_cb_bar_group(parse_t *parse, plot_t *plot);

extern parse_cb_entry_t parse_cb_top_props[];
extern const int parse_cb_top_props_count;

void parse_top_property(parse_t *parse, plot_t *plot);

// Top function parsing and table
extern parse_cb_entry_t parse_cb_top_funcs[];
extern const int parse_cb_top_funcs_count;

// Top func parse functions
void parse_top_func(parse_t *parse, plot_t *plot);
void parse_cb_print(parse_t *parse, plot_t *plot);
void parse_cb_reset(parse_t *parse, plot_t *plot);
void parse_cb_save_fig(parse_t *parse, plot_t *plot);
void parse_cb_save_legend(parse_t *parse, plot_t *plot);
void parse_cb_create_fig(parse_t *parse, plot_t *plot);
void parse_cb_set_hatch_scheme(parse_t *parse, plot_t *plot);
void parse_cb_set_color_scheme(parse_t *parse, plot_t *plot);
void parse_cb_test_hatch(parse_t *parse, plot_t *plot);
void parse_cb_test_color(parse_t *parse, plot_t *plot);
void parse_cb_dump(parse_t *parse, plot_t *plot);

#define PARSE_SPEC_FLOAT "fFeEgGaA"
#define PARSE_SPEC_STR   "sp"
#define PARSE_SPEC_INT32 "dixXuo"
#define PARSE_SPEC_ALL   "diuoxXfFeEgGaAcspn"

void parse_print_prop(parse_t *parse, plot_t *plot, buf_t *buf, const char *name, const char *fmt);
void parse_print_fmt(parse_t *parse, plot_t *plot, buf_t *buf, const char *str);

void parse_report_pos(parse_t *parse);
void parse_print(parse_t *parse);

#endif