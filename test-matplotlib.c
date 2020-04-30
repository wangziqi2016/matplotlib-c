
#include "matplotlib.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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

void test_color() {
  printf("========== test_color ==========\n");
  uint32_t color;
  char buf[32];
  printf("Step 1: Test COLOR_GEN() and color_str()\n");
  color = COLOR_GEN(255, 255, 255);
  color_str(color, buf);
  printf("Color 0x%X Str \"%s\"\n", color, buf);
  color = COLOR_GEN(300, 300, 300); // Overflow, but should wrap back (0x2C)
  color_str(color, buf);
  printf("Color 0x%X Str \"%s\"\n", color, buf);
  color = COLOR_GEN(0xAB, 0xCD, 0xEF); 
  color_str(color, buf);
  printf("Color 0x%X Str \"%s\"\n", color, buf);
  assert(COLOR_R(color) == 0xAB);
  assert(COLOR_G(color) == 0xCD);
  assert(COLOR_B(color) == 0xEF);
  printf("Step 2: Test color_find_scheme()\n");
  color_scheme_t *scheme = NULL;
  scheme = color_find_scheme("red");
  assert(scheme->base == color_scheme_red);
  color_scheme_print(scheme);
  scheme = color_find_scheme("mixed");
  assert(scheme->base == color_scheme_mixed);
  color_scheme_print(scheme);
  printf("Pass\n");
  return;
}

void test_py() {
  printf("========== test_py ==========\n");
  assert(py_get_instance_count() == 0);
  py_t *py = py_init();
  // Print a simple string
  py_run(py, "print('Hello, Python!')"); // Python print() will append a new line automatically
  // Uncomment this to see an error
  //py_run(py, "asdf");
  assert(py_get_instance_count() == 1);
  // Test whether ref counter works correctly
  py_t *py2 = py_init();
  assert(py_get_instance_count() == 2);
  py_run(py2, "import sys\nsys.stdout.write('Hello, Python again!\\n')");
  py_free(py2);
  assert(py_get_instance_count() == 1);
  py_free(py);
  assert(py_get_instance_count() == 0);
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
  printf("  Size %d strlen %d\n", buf_get_size(buf), buf_strlen(buf)); // Test stat get function
  buf_print(buf, 1);
  printf("Step 2: Test reset and printf\n");
  buf_reset(buf);
  buf_print(buf, 1);
  buf_printf(buf, "  Size %d strlen %d\n", buf_get_size(buf), buf_strlen(buf));
  buf_print(buf, 1);
  printf("Step 3: Test printf with super long string for the alternate code path\n");
  buf_reset(buf);
  buf_printf(buf, "%0*d", BUF_INIT_SIZE * 2 + 1, 0); // %0*d means left-fill with zeros
  buf_print(buf, 1); // Expect (BUF_INIT_SIZE * 2 + 1) "0"
  assert(buf_strlen(buf) == (BUF_INIT_SIZE * 2 + 1));
  printf("Step 4: Test dump\n");
  const char *filename = "test_buf_dump.txt";
  buf_dump(buf, filename);
  struct stat stat;
  lstat(filename, &stat);
  printf("File info: size %d\n", (int)stat.st_size);
  assert(stat.st_size == buf_strlen(buf));
  int rm_ret = remove(filename);
  assert(rm_ret == 0);
  printf("Remove file \"%s\"\n", filename);
  printf("Step 5: Test append color\n");
  buf_reset(buf);
  buf_append_color(buf, COLOR_GEN(0x12, 0x34, 0x56));
  buf_append_color(buf, COLOR_GEN(0xff, 0xff, 0xff));
  buf_append_color(buf, COLOR_GEN(0x55, 0xaa, 0x55));
  buf_print(buf, 1);
  assert(buf_strlen(buf) == COLOR_STRLEN * 3); // Three color strings
  buf_free(buf);
  printf("Pass\n");
  return;
}

void test_bar_type() {
  printf("========== test_bar_type ==========\n");
  plot_t *plot = plot_init();
  plot_add_bar_type(plot, "type1", COLOR_GEN(0x12, 0x34, 0x56), 'x');
  plot_add_bar_type(plot, "type2", COLOR_GEN(0xff, 0xff, 0xff), 'y');
  plot_add_bar_type(plot, "type3", COLOR_GEN(0xab, 0xcd, 0xef), 'z');
  // Uncomment this to test duplicated label
  //plot_add_bar_type(plot, "type3", COLOR_GEN(0xab, 0xcd, 0xef), 'z');
  bar_type_t *type;
  type = plot_find_bar_type(plot, "type2");
  printf("Type Label %s color 0x%08X hatch %c used %d\n", type->label, type->color, type->hatch, type->used);
  assert(type->color == 0x00FFFFFF && type->hatch == 'y');
  assert(streq(type->label, "type2") == 1);
  assert(type->used == 0);
  type->used = 1;
  // Test dup() function
  bar_type_t *dup = bar_type_dup(type);
  printf("Dup Label %s color 0x%08X hatch %c used %d\n", dup->label, dup->color, dup->hatch, dup->used);
  assert(dup->color == 0x00FFFFFF && dup->hatch == 'y');
  assert(streq(dup->label, "type2") == 1);
  assert(dup->label != type->label); // Label must be anew
  assert(dup->used == 0); // This should not be copied
  bar_type_free(dup);
  // Finish all tests
  plot_free(plot);
  printf("Pass\n");
  return;
}

int main(int argc, char **argv) {
  int valgrind_flag = 0;
  for(int i = 1;i < argc;i++) {
    if(streq(argv[i], "--valgrind") == 1) valgrind_flag = 1;
  }
  if(valgrind_flag == 1) printf("[main] Skipping some tests under Valgrind\n");
  printf("========== test-matplotlib ==========\n");
  test_fp_power10();
  test_fp_trim();
  test_fp_print();
  test_color();
  test_bar_type();
  if(valgrind_flag == 0) test_py();
  test_buf();
  printf("All test passed!\n");
  return 0;
}