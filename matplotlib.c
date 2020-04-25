
#include "matplotlib.h"

//* fp_*

double fp_power10(int num) {
  double ret = 1.0;
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
//   - Negative frac_count will remove numbers left to the decimal point
char *fp_print(double num, int frac_count) {
  int buf_size = FP_BUF_SIZE;
  int count = 0;
  char *buf;
  //if(frac_count < 0) 
  do {
    buf = (char *)malloc(buf_size);
    SYSEXPECT(buf != NULL);
    int ret = snprintf(buf, buf_size, "%.*f", frac_count, num);
    if(ret > 0 && ret < buf_size) {
      break;
    } else if(ret < 0) {
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
  return buf;
}