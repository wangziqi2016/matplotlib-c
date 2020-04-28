
#include "matplotlib.h"

//* fp_*

double fp_power10(int num) {
  // If num == 0 if will return 1.0
  double ret = 1.0f;
  if(num >= 0) {
    for(int i = 0;i < num;i++) {
      ret *= 10.0;
    }
  } else {
    num = -num;
    for(int i = 0;i < num;i++) {
      ret /= 10.0;
    }
  }
  return ret;
}

// frac_count specifies the number of digits after the decimal point
//   - If set to 0 then we print integer
//   - How fp numbers are rounded is dependent on printf() implementation
//   - Negative frac_count will cause digits left to the decimal point to be zero
char *fp_print(double num, int frac_count) {
  int buf_size = FP_BUF_SIZE;
  int count = 0;
  char *buf;
  int printf_ret;
  // Append digit "0" to the string after printf
  int append_zero_count = 0;
  if(frac_count < 0) {
    num /= fp_power10(-frac_count);
    append_zero_count = -frac_count;
    frac_count = 0;
  }
  assert(append_zero_count >= 0);
  assert(append_zero_count == 0 || frac_count == 0);
  do {
    buf = (char *)malloc(buf_size);
    SYSEXPECT(buf != NULL);
    printf_ret = snprintf(buf, buf_size, "%.*f", frac_count, num);
    // Since we may append extra zeros after this, also count them into ret
    if(printf_ret > 0 && (printf_ret + append_zero_count) < buf_size) {
      break;
    } else if(printf_ret <= 0) {
      // Directly push binary into the stack
      error_exit("Encoding error occurred while printing floating point number 0x%lX\n", *(uint64_t *)&num);
    }
    free(buf);
    buf_size *= 2;
    // Set these two to make sure we always exit from the loop
    buf = NULL;
    count++;
  } while(count < FP_MAX_ITER);
  if(buf == NULL) error_exit("Internal error: maximum iteration count reached (%d)\n", count);
  // Append zero to the end (there must be no decimal point)
  assert(printf_ret + append_zero_count < buf_size);
  // Corner case: If the resulting number is already zero, we do not append anything
  if(streq(buf, "0") == 0) {
    for(int i = 0;i < append_zero_count;i++) {
      strcat(buf, "0");
    }
  }
  return buf;
}

// Remove trailing zeros after the decimal point
//   - If no decimal point then directly exit
//   - If all zeros after the decimal point, we also remove the point itself
char *fp_rtrim(char *buf) {
  char *decimal = strchr(buf, '.');
  if(decimal == NULL) return buf; // No change
  char *p = decimal + 1;
  assert(*p != '\0'); // Can't be trailing decimal
  char *q = buf + strlen(buf) - 1;
  assert(q[1] == '\0');
  // Find first non-zero from the end of the string
  while(*q == '0') {
    q--;
    assert(q >= decimal);
  }
  // Overwrite decimal if all digits after it are zero; Otherwise do not overwrite
  if(q != decimal) q++;
  *q = '\0'; // Terminate the string here
  return buf;
}

//* color_*

uint32_t color_scheme_mixed[] = {
  COLOR_GEN(0xff, 0x66, 0x00),
  COLOR_GEN(0x33, 0x66, 0x99),
  COLOR_GEN(0x02, 0x84, 0x82),
  COLOR_GEN(0xcc, 0x99, 0x00),
  COLOR_GEN(0xcc, 0xcc, 0xff),
  COLOR_GEN(0x9c, 0x9f, 0x84),
};

uint32_t color_scheme_red[] = {
  COLOR_GEN(0xfd, 0xd4, 0x9e),
  COLOR_GEN(0xfd, 0xbb, 0x84),
  COLOR_GEN(0xfc, 0x8d, 0x59),
  COLOR_GEN(0xef, 0x65, 0x48),
  COLOR_GEN(0xd7, 0x30, 0x1f),
  COLOR_GEN(0xb3, 0x00, 0x00),
  COLOR_GEN(0x7f, 0x00, 0x00),
};

extern color_scheme_t color_schemes[] = {
  COLOR_SCHEME_GEN("mixed", color_scheme_mixed),
  COLOR_SCHEME_GEN("red", color_scheme_red),
};

// This function is not re-entrant; Only one instance can be called before the next
void color_str(uint32_t color, char *buf) {
  sprintf(buf, "#%02X%02X%02X", COLOR_R(color), COLOR_G(color), COLOR_B(color));
  return;
}

// Returns NULL if the name does not exist; Otherwise return the pointer to the scheme
color_scheme_t *color_find_scheme(const char *name) {
  for(int i = 0;i < (int)(sizeof(color_schemes) / sizeof(color_scheme_t));i++) {
    if(streq(color_schemes[i].name, name) == 1) {
      return color_schemes + i;
    }
  }
  return NULL;
}

//* buf_t

buf_t *buf_init_sz(int sz) {
  if(sz <= 0) {
    error_exit("Initialized size must be non-negative (see %d)\n", sz);
  }
  buf_t *buf = (buf_t *)malloc(sizeof(buf_t));
  SYSEXPECT(buf != NULL);
  memset(buf, 0x00, sizeof(buf_t));
  buf->size = 1; // Note that this includes the trailing zero
  buf->capacity = sz;
  buf->data = (char *)malloc(sz);
  SYSEXPECT(buf->data != NULL);
  memset(buf->data, 0x00, sz);
  return buf;
}

buf_t *buf_init() {
  return buf_init_sz(BUF_INIT_SIZE);
}

void buf_free(buf_t *buf) {
  free(buf->data);
  free(buf);
  return;
}

// Clear all content, but do not realloc storage
void buf_reset(buf_t *buf) {
  assert(buf->capacity > 0);
  buf->size = 1;
  buf->data[0] = '\0';
  return;
}

// Reallocate storage, doubling the buffer capacity
// This call has no effect if target if smaller than current capacity
void buf_realloc(buf_t *buf, int target) {
  assert(buf->size <= buf->capacity);
  if(target <= buf->capacity) {
    return;
  }
  // Always allocate power of two
  while(buf->capacity < target) buf->capacity *= 2;
  void *old_data = buf->data;
  buf->data = (char *)malloc(buf->capacity);
  SYSEXPECT(buf->data != NULL);
  // This includes the trailing zero
  memcpy(buf->data, old_data, buf->size);
  free(old_data);
  return;
}

// Append the string to the end of the buffer
void buf_append(buf_t *buf, const char *s) {
  int len = strlen(s);
  // This will likely not cause a realloc
  buf_realloc(buf, buf->size + len); 
  // Start from the last char, copy includes the trailing zero
  memcpy(buf->data + buf->size - 1, s, len + 1);
  buf->size += len;
  return;
}

// Concatenate the second arg after the first one and frees the second one
void buf_concat(buf_t *buf, buf_t *s) {
  buf_append(buf, s->data);
  buf_free(s);
  return;
}

void buf_printf(buf_t *buf, const char *fmt, ...) {
  va_list(args);
  va_start(args, fmt);
  char temp[BUF_INIT_SIZE];
  int expected = vsnprintf(temp, BUF_INIT_SIZE, fmt, args);
  if(expected >= 0 && expected < BUF_INIT_SIZE) {
    buf_append(buf, temp);
    return;
  }
  // Start a new arg, since the previous one may have been destroyed by the printf
  va_list(args2);
  va_start(args2, fmt);
  char *temp2 = (char *)malloc(expected + 1);
  SYSEXPECT(temp2 != NULL);
  int expected2 = vsnprintf(temp2, expected + 1, fmt, args2);
  assert(expected2 == expected);
  buf_append(buf, temp2);
  free(temp2);
  return;
}

void buf_stat_print(buf_t *buf, int content) {
  printf("---------- buf_t stat ----------\n");
  printf("Size %d cap %d data 0x%p\n", buf->size, buf->capacity, buf->data);
  if(content == 1) {
    printf("%s\n", buf->data);
  }
  return;
}

void buf_dump(buf_t *buf, const char *filename) {
  FILE *fp = fopen(filename, "w");
  SYSEXPECT(fp != NULL);
  // Do not write the trailing zero to the file
  int ret = fwrite(buf->data, buf->size - 1, 1, fp);
  if(ret != 1) {
    printf("Error writing to the file %s\n", filename);
    error_exit("Cannot write file\n");
  }
  fclose(fp);
  return;
}

//* bar_t

bar_t *bar_init() {
  bar_t *bar = (bar_t *)malloc(sizeof(bar_t));
  SYSEXPECT(bar != NULL);
  memset(bar, 0x00, sizeof(bar_t));
  return bar;
}

void bar_free(bar_t *bar) {
  if(bar->text != NULL) free(bar->text);
  free(bar);
  return;
}