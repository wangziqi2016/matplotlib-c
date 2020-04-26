
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

//* buf_t

buf_t *buf_init() {
  buf_t *buf = (buf_t *)malloc(sizeof(buf_t *));
  SYSEXPECT(buf != NULL);
  memset(buf, 0x00, sizeof(buf_t));
  buf->size = 1; // Note that this includes the trailing zero
  buf->capacity = BUF_INIT_SIZE;
  buf->data = (char *)malloc(BUF_INIT_SIZE);
  SYSEXPECT(buf->data != NULL);
  memset(buf->data, 0x00, BUF_INIT_SIZE);
  return buf;
}

void buf_free(buf_t *buf) {
  free(buf->data);
  free(buf);
  return;
}

// Reallocate storage, doubling the buffer capacity
void buf_realloc(buf_t *buf) {
  assert(buf->size <= buf->capacity);
  buf->capacity *= 2;
  void *old_data = buf->data;
  buf->data = (char *)malloc(buf->capacity);
  SYSEXPECT(buf->data != NULL);
  // This includes the trailing zero
  memcpy(buf->data, old_data, buf->size);
  return;
}

// Append the string to the end of the buffer
void buf_append(buf_t *buf, const char *s) {
  int len = strlen(s);
  // Loop until we reach the intended capacity
  while(1) {
    // This might be expensive when appending long str, but we optimize for short appends
    if(buf->size + len > buf->capacity) {
      buf_realloc(buf); 
    }
  }
  // Start from the last char, copy includes the trailing zero
  memcpy(buf->data + buf->size - 1, s, len + 1);
  buf->size += len;
  return;
}

void buf_print(buf_t *buf, int content) {
  printf("---------- buf_t ----------\n");
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