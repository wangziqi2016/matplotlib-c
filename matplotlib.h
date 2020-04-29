
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

#include "Python.h"

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
#define COLOR_SIZE sizeof(uint32_t)

// Macros for extracting color components
#define COLOR_R(x) ((x >> 16) & 0xFF)
#define COLOR_G(x) ((x >> 8) & 0xFF)
#define COLOR_B(x) ((x >> 0) & 0xFF)
// Composing a color using RGB components
#define COLOR_GEN(r, g, b) (((r & 0xFF) << 16) | ((g & 0xFF) << 8) | ((b & 0xFF) << 0))

typedef struct {
  const char *name;   // Name of the scheme
  uint32_t *base;     // Points to the array
  int item_count;     // Number of items in the base array
} color_scheme_t;

extern uint32_t color_scheme_mixed[];
extern uint32_t color_scheme_red[];

// This macro generates an entry in color_schemes
#define COLOR_SCHEME_GEN(name, base) {name, base, sizeof(base) / COLOR_SIZE}

extern color_scheme_t color_schemes[];

void color_str(uint32_t color, char *buf); // Returns RGB color code
color_scheme_t *color_find_scheme(const char *name);  // Using string name to locate the color_scheme_t object

void color_scheme_print(color_scheme_t *scheme);

//* py_* - Python interpreter

typedef struct {
  uint64_t padding; // Avoid allocating 0 bytes (some malloc may not behave correctly)
} py_t;

py_t *py_init();
void py_free(py_t *py);
void py_run(const char *s);

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

inline static int buf_get_size(buf_t *buf) { return buf->size; }
inline static int buf_get_capacity(buf_t *buf) { return buf->capacity; }
inline static char *buf_c_str(buf_t *buf) { return buf->data; }
inline static int buf_strlen(buf_t *buf) { return buf->size - 1; }

void buf_reset(buf_t *buf);
void buf_realloc(buf_t *buf, int target);
void buf_append(buf_t *buf, const char *s);
void buf_concat(buf_t *buf, buf_t *s);
void buf_printf(buf_t *buf, const char *fmt, ...);

void buf_print(buf_t *buf, int content);
void buf_dump(buf_t *buf, const char *filename);

//* bar_t - Bar object

typedef struct {
  double value;   // Height of the bar
  double bottom;  // Non-zero means we draw stacked bar
  double pos;     // Offset in the horizontal direction
  char hatch;     // Hatch (filling pattern)
  uint32_t color; // RGB color
  char *text;     // Optional text (if NULL then use value as text or do not care)
} bar_t;

bar_t *bar_init();
void bar_free(bar_t *bar);

//* plot_t - Plotting function

extern const char *plot_preamble; // This is added to the buffer on plot init

typedef struct {
  py_t *py;
  buf_t *buf;
} plot_t;

plot_t *plot_init();
void polt_free(plot_t *plot);

void plot_create_fig(double width, double height);

#endif