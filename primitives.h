
#ifndef _MATPLOTLIB_C_PRIMITIVES
#define _MATPLOTLIB_C_PRIMITIVES

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

// This header file defines the primitives shapes that can be drawn

//* error handling

// Error reporting and system call assertion
#define SYSEXPECT(expr) do { if(!(expr)) { perror(__func__); exit(1); } } while(0)
#define error_exit(fmt, ...) do { fprintf(stderr, "%s error: " fmt, __func__, ##__VA_ARGS__); exit(1); } while(0);
#ifndef NDEBUG
#define dbg_printf(fmt, ...) do { fprintf(stderr, fmt, ##__VA_ARGS__); } while(0);
#else
#define dbg_printf(fmt, ...) do {} while(0);
#endif

// Position of a point. If this is given relative to a plot, it always starts at bottom left of the rectangular
typedef struct {
  double x;
  double y;
} point_t;

typedef struct {
  double width;
  double height;
} plot_t;

typedef struct {
  // There is some redundancy, but we store them for simplicity
  point_t tl; // Top left
  point_t tr; // Top right
  point_t bl; // Bottom left
  point_t br; // Bottom right
  // Also redundant
  double width;
  double height;
  // Other properties
  uint32_t fill_color;
  uint32_t edge_color;
  int edge_widith;
  char hatch;
} bar_t;

#endif