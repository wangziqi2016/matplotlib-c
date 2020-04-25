
#ifndef _MATPLOTLIB_H
#define _MATPLOTLIB_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

//* fstr_* - String processing of floating point numbers

#define FSTR_CUT_AFTER_RADIX     0  // Leave certain number of digits after the radix point
#define FSTR_CUT_SIG_DIGITS      1  // Significant digits

char *fstr_cut(double num, int type, int arg); // Cutting the string in different ways

#endif