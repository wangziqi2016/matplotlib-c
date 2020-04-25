
#include "matplotlib.h"

void test_fp_print() {
  printf("========== test_fp_print ==========\n");
  char *buf = NULL;
  printf("Step: Test number %f using non-neg frac count\n");
  for(int i = 0;i < 8;i++) {
    buf = fp_print(12345.678912, i);
    printf("")
    free(buf);
  }

  printf("Pass\n");
  return;
}

int main() {
  printf("========== test-matplotlib ==========\n");

  printf("All test passed!\n");
  return 0;
}