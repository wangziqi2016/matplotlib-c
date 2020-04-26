
#include "matplotlib.h"

/* Use the following as test function template
void test_xxx() {
  printf("========== test_xxx ==========\n");

  printf("Pass\n");
  return;
}
*/

void test_fp_power10() {
  printf("========== test_fp_power10 ==========\n");
  for(int i = -5;i < 5;i++) {
    printf("power = %d; num = %f\n", i, fp_power10(i));
  }
  printf("Pass\n");
  return;
}

void test_fp_trim() {
  printf("========== test_fp_trim ==========\n");
  const double num = 12345.678912;
  printf("Step 1: Use %f and fp_printf() for testing\n", num);
  for(int i = -6;i < 10;i++) {
    char *buf = fp_print(num, i);
    // Note: Must use two separate printf, since we need to evaluate trim after the print
    printf("num = \"%s\"", buf);
    printf(", after trim = \"%s\"\n", fp_rtrim(buf));
    free(buf);
  }
  printf("Step 2: Corner cases\n");
  char buf[32], buf2[32];
  strcpy(buf, "0.00000"); strcpy(buf2, "0.00000");
  printf("num = \"%s\", after trim = \"%s\"\n", buf, fp_rtrim(buf2));
  printf("Pass\n");
  return;
}

void test_fp_print() {
  printf("========== test_fp_print ==========\n");
  char *buf = NULL;
  const double num = 12345.678912;
  printf("Step 1: Test number %f using non-neg frac count\n", num);
  for(int i = 0;i < 8;i++) {
    buf = fp_print(num, i);
    printf("frac_count = %d: \"%s\"\n", i, buf);
    free(buf);
  }
  printf("Step 2: Test number %f using negative frac count\n", num);
  for(int i = -5;i < 0;i++) {
    buf = fp_print(num, i);
    printf("frac_count = %d: \"%s\"\n", i, buf);
    free(buf);
  }
  printf("Pass\n");
  return;
}

void test_buf() {
  printf("========== test_buf ==========\n");
  printf("Step 1: Testing basic append and concat\n");
  buf_t *buf = buf_init_sz(1); // Starting from 1 to test code path for realloc
  buf_append(buf, "wangziqi");
  buf_append(buf, "");
  buf_append(buf, "2013");
  buf_t *buf2 = buf_init(2);
  buf_append(buf2, " This is a very long string that may require several loops for realloc\n");
  buf_concat(buf, buf2); // buf2 is freed after this point
  buf_print(buf, 1);
  buf_free(buf);
  printf("Pass\n");
  return;
}

int main() {
  printf("========== test-matplotlib ==========\n");
  test_fp_power10();
  test_fp_trim();
  test_fp_print();
  test_buf();
  printf("All test passed!\n");
  return 0;
}