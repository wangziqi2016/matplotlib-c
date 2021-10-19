
#ifndef _MATPLOTLIB_C_PRIMITIVES
#define _MATPLOTLIB_C_PRIMITIVES

// This header file defines the primitives shapes that can be drawn

// Position of a point. If this is given relative to a plot, it always starts at bottom left of the rectangular
typedef struct {
  double x;
  double y;
} point_t;

typedef struct {
  double width;
  double height;
} plot_t;

#endif