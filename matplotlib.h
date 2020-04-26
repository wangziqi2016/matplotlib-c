
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

//* buf_* - String buffer

#define BUF_INIT_SIZE 256

typedef struct {
  char *data;
  int capacity; // Actual number of usable bytes
  int size;     // Number of used bytes, including trailing zero
} buf_t;

buf_t *buf_init();
void buf_free(buf_t *buf);

inline static int buf_get_size(buf_t *buf) { return buf->size; }
inline static int buf_get_capacity(buf_t *buf) { return buf->capacity; }
inline static char *buf_c_str(buf_t *buf) { return buf->data; }
inline static int buf_strlen(buf_t *buf) { return buf->size - 1; }

void buf_realloc(buf_t *buf);
void buf_append(buf_t *buf, const char *s);

#endif