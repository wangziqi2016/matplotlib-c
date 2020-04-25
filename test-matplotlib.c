
#include "matplotlib.h"

void test_fp_print() {
  printf("========== test_fp_print ==========\n");
  char *buf = NULL;
  const double num = 12345.678912;
  printf("Step: Test number %f using non-neg frac count\n", num);
  for(int i = 0;i < 8;i++) {
    buf = fp_print(num, i);
    printf("frac_count = %d: \"%s\"\n", i, buf);
    free(buf);
  }
  // TODO: NEGATIVE FRAC COUNT
  printf("Pass\n");
  return;
}

int main() {
  printf("========== test-matplotlib ==========\n");
  test_fp_print();
  printf("All test passed!\n");
  return 0;
}