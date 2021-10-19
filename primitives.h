
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

//* String functions

inline static int streq(const char *a, const char *b) { return strcmp(a, b) == 0; }
char *strclone(const char *s); // Duplicate a string (strdup is already used)

// Position of a point. If this is given relative to a plot, it always starts at bottom left of the rectangular
typedef struct {
  double x;
  double y;
} point_t;

typedef struct {
  double width;
  double height;
} plot_t;

//* bar_t

// Masks indicating whether a position is set by user ("1" bit if set by user, "0" if derived)
#define BAR_POS_MASK_TL     0x00000001UL
#define BAR_POS_MASK_TR     0x00000002UL
#define BAR_POS_MASK_BL     0x00000004UL
#define BAR_POS_MASK_BR     0x00000008UL
#define BAR_POS_MASK_WIDTH  0x00000010UL
#define BAR_POS_MASK_HEIGHT 0x00000020UL

typedef struct {
  // String name of the bar, for error reporting; NULL means no name which is also valid
  // but will make the bar non-addressable
  char *name;
  // There is some redundancy, but we store them for simplicity
  point_t tl; // Top left
  point_t tr; // Top right
  point_t bl; // Bottom left
  point_t br; // Bottom right
  // Also redundant
  double width;
  double height;
  // Mask for checking whether users have set the bar correctly
  uint32_t pos_mask;
  // Other properties
  uint32_t fill_color;
  uint32_t edge_color;
  int edge_widith;
  char hatch;
} bar_t;

bar_t *bar_init();      // Initialize an empty bar that cannot be drawn
bar_t *bar_init_name(char *name); // Initialize a bar with a name
void bar_free(bar_t *bar);

// Bar coordinator setters (users can set them freely, and we check consistency during validation)
void bar_set_tl(bar_t *bar, point_t tl);
void bar_set_tr(bar_t *bar, point_t tr);
void bar_set_bl(bar_t *bar, point_t bl);
void bar_set_br(bar_t *bar, point_t br);

void bar_set_width(bar_t *bar, double width);
void bar_set_height(bar_t *bar, double height);

// Either passing the validation and returns, or report error and never return
void bar_validate(bar_t *bar);

#endif