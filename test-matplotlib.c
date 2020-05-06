
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
  color_scheme_print(scheme, 1);
  scheme = color_find_scheme("mixed");
  assert(scheme->base == color_scheme_mixed);
  color_scheme_print(scheme, 1);
  printf("Pass\n");
  return;
}

void test_color_init_file() {
  printf("========== test_color_init_file ==========\n");
  color_scheme_t *scheme = color_scheme_init_file("./conf/test_color_init_file.txt");
  assert(scheme != NULL);
  color_scheme_print(scheme, 1);
  color_scheme_free(scheme);
  printf("Pass\n");
  //exit(1);
  return;
}

void test_hatch_init_file() {
  printf("========== test_color_init_file ==========\n");
  hatch_scheme_t *scheme = hatch_scheme_init_file("./conf/test_hatch_init_file.txt");
  assert(scheme != NULL);
  hatch_scheme_print(scheme, 1);
  hatch_scheme_free(scheme);
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

void test_vec() {
  printf("========== test_vec ==========\n");
  vec_t *vec = vec_init();
  for(int i = 0;i < 1000;i++) {
    vec_append(vec, (void *)(uint64_t)i);
  }
  vec_print(vec);
  for(int i = 999;i >= 0;i--) {
    assert(vec_at(vec, i) == (void *)(uint64_t)i);
  }
  vec_free(vec);
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

void test_plot_legend() {
  printf("========== test_plot_legend ==========\n");
  plot_t *plot = plot_init();
  // Adding bar types
  plot_add_bar_type(plot, "Category 1", color_scheme_mixed[0], hatch_scheme_default[0]);
  plot_add_bar_type(plot, "Category 2", color_scheme_mixed[1], hatch_scheme_default[1]);
  plot_add_bar_type(plot, "Category 3", color_scheme_mixed[2], hatch_scheme_default[2]);
  plot_add_bar_type(plot, "Category 4", color_scheme_mixed[3], hatch_scheme_default[3]);
  plot_add_bar_type(plot, "Category 5", color_scheme_mixed[4], hatch_scheme_default[4]);
  // Note that this should print nothing about the legend, since we create another object
  plot_print(plot, 1); 
  // Uncomment the following to reveal error
  //plot_set_legend_rows(plot, -2);
  plot_set_legend_rows(plot, 1);
  plot_save_legend(plot, "test_legend_1.pdf");
  plot_set_legend_rows(plot, 100);
  plot_save_legend(plot, "test_legend_2.pdf");
  plot_set_legend_rows(plot, 3);
  plot_save_legend(plot, "test_legend_3.pdf");
  plot_free(plot);
  printf("Pass\n");
  return;
}

void test_plot_color_test() {
  printf("========== test_plot_color_test ==========\n");
  plot_t *plot = plot_init();
  plot->param.color_scheme = color_find_scheme("mixed");
  plot->param.color_offset = 0;
  plot_save_color_test(plot, "test_color_1.pdf");
  plot_print(plot, 1);
  plot_free(plot);
  printf("Pass\n");
  return;
}

void test_parse_getchar() {
  printf("========== test_parse_getchar ==========\n");
  // Line 1, 3, 4 has contents; 6 lines in total
  const char *s = "first line\n\nsecond line\nthird line\n\n\n";
  parse_t *parse = parse_init(s);
  buf_t *buf = buf_init();
  char ch;
  while((ch = parse_getchar(parse)) != '\0') {
    buf_printf(buf, "%c", ch);
    int line = parse_get_line(parse);
    int col = parse_get_col(parse);
    if(ch == '\n') printf("char \\n line %d col %d\n", line, col);
    else printf("char %c line %d col %d\n", ch, line, col);
  }
  buf_print(buf, 1);
  parse_print(parse);
  assert(streq(buf_c_str(buf), s) == 1);
  assert(parse_get_line(parse) == 7);
  assert(parse_get_col(parse) == 0);
  // Test whether getchar stops at the end
  assert(parse_getchar(parse) == '\0');
  assert(parse_getchar(parse) == '\0');
  buf_free(buf);
  parse_free(parse);
  printf("Pass\n");
  return;
}

void test_parse_until() {
  printf("========== test_parse_until ==========\n");
  const char *s = "  first-line;\n\n    second-line   \n\n 3  .\n";
  parse_t *parse = parse_init(s);
  char *ret;
  ret = parse_until(parse, ';');
  printf("ret = \"%s\"\n", ret);
  assert(streq(ret, "first-line") == 1);
  free(ret);
  ret = parse_until(parse, '\n');
  printf("ret = \"%s\"\n", ret);
  assert(streq(ret, "second-line") == 1);
  free(ret);
  ret = parse_until(parse, '.');
  printf("ret = \"%s\"\n", ret);
  assert(streq(ret, "3") == 1);
  free(ret);
  ret = parse_until(parse, ','); // Non-existing char before string end
  printf("ret = 0x%p\n", ret);
  assert(ret == NULL);
  free(ret);
  parse_free(parse);
  printf("Pass\n");
  return;
}

void test_parse_ident() {
  printf("========== test_parse_ident ==========\n");
  const char *s = "   ident1 \n ident 2 \n\n ;;;;; \n";
  parse_t *parse = parse_init(s);
  char *ret;
  ret = parse_get_ident(parse);
  printf("ret = \"%s\"\n", ret);
  assert(streq(ret, "ident1") == 1);
  free(ret);
  ret = parse_get_ident(parse);
  printf("ret = \"%s\"\n", ret);
  assert(streq(ret, "ident") == 1);
  free(ret);
  ret = parse_get_ident(parse);
  printf("ret = 0x%p\n", ret);
  assert(ret == NULL);
  // Skip "2"
  assert(parse_peek(parse) == '2');
  parse_getchar(parse);
  ret = parse_get_ident(parse);
  printf("ret = 0x%p\n", ret);
  assert(ret == NULL);
  assert(parse_peek(parse) == ';');
  for(int i = 0;i < 5;i++) {
    parse_getchar(parse); // Skip all five ';'
  }
  ret = parse_get_ident(parse);
  printf("ret = 0x%p\n", ret);
  assert(ret == NULL);
  parse_free(parse);
  printf("Pass\n");
  return;
}

void test_parse_expect() {
  printf("========== test_parse_expect ==========\n");
  const char *s = "   ; ident {} \n ,,, \n\n\n";
  parse_t *parse = parse_init(s);
  char *ret;
  parse_expect_char(parse, ';');
  ret = parse_get_ident(parse);
  assert(streq(ret, "ident") == 1);
  free(ret);
  parse_expect_char(parse, '{');
  parse_expect_char(parse, '}');
  parse_expect_char(parse, ',');
  parse_expect_char(parse, ',');
  // This should work
  parse_expect_char_opt(parse, '\n');
  parse_expect_char_opt(parse, ',');
  // This will not work since we jump over white spaces
  //parse_expect_char(parse, '\n');
  // These two, however, works since the char is optional
  parse_expect_char_opt(parse, '\n');
  parse_expect_char_opt(parse, '\0');
  assert(parse_peek(parse) == '\0');
  parse_free(parse);
  printf("Pass\n");
  return;
}

void test_parse_str() {
  printf("========== test_parse_str ==========\n");
  const char *s = " \"This is a string\"  \"This is \\\"string\\\"\"  \" This is\\nnew line\"";
  parse_t *parse = parse_init(s);
  char *ret;
  ret = parse_get_str(parse);
  printf("ret = \"%s\"\n", ret);
  assert(streq(ret, "This is a string") == 1);
  free(ret);
  ret = parse_get_str(parse);
  printf("ret = \"%s\"\n", ret);
  assert(streq(ret, "This is \\\"string\\\"") == 1);
  free(ret);
  ret = parse_get_str(parse);
  printf("ret = \"%s\"\n", ret);
  assert(streq(ret, " This is\\nnew line") == 1);
  free(ret);
  // Uncomment this to test error - no quotation mark
  //ret = parse_get_str(parse);
  // Uncomment the following to test error - unclosed string
  //const char *s2 = " \"    This is an unclosed string. \\n ";
  //parse_t *parse2 = parse_init(s2);
  //ret = parse_get_str(parse2);
  // Finish test
  parse_free(parse);
  printf("Pass\n");
  return;
}

void test_parse_double() {
  printf("========== test_parse_double ==========\n");
  const char *s = " 123.4567 0.0 123 .2 xyzw";
  parse_t *parse = parse_init(s);
  // Note: If there is f suffix then it is float; Otherwise it is double
  printf("sizeof f suffix: %d; no suffix: %d; l suffix: %d\n", 
    (int)sizeof(123.0f), (int)sizeof(123.0), (int)sizeof(123.0l));
  double ret;
  ret = parse_get_double(parse);
  printf("ret = %f\n", ret);
  assert(ret == 123.4567);
  ret = parse_get_double(parse);
  printf("ret = %f\n", ret);
  assert(ret == 0.0f);
  ret = parse_get_double(parse);
  printf("ret = %f\n", ret);
  assert(ret == 123.0);
  ret = parse_get_double(parse);
  printf("ret = %f\n", ret);
  assert(ret == 0.2);
  // Uncomment the following to reveal error
  //ret = parse_get_double(parse);
  // Finish test
  parse_free(parse);
  printf("Pass\n");
  return;
}

void test_parse_int() {
  printf("========== test_parse_double ==========\n");
  const char *s = " 123 4567 0 123 .2 xyzw";
  parse_t *parse = parse_init(s);
  int64_t ret;
  ret = parse_get_int64(parse);
  printf("ret = %ld\n", ret);
  assert(ret == 123l);
  ret = parse_get_int64(parse);
  printf("ret = %ld\n", ret);
  assert(ret == 4567l);
  ret = parse_get_int64(parse);
  printf("ret = %ld\n", ret);
  assert(ret == 0l);
  // Uncomment the following and comment out the next line to reveal range error
  //ret = parse_get_int64_range(parse, 100, 122);
  ret = parse_get_int64_range(parse, 100, 123);
  // Uncomment the following to reveal error
  //ret = parse_get_int64(parse);
  // Finish test
  parse_free(parse);
  printf("Pass\n");
  return;
}

void test_parse_skip_space() {
  printf("========== test_parse_skip_space ==========\n");
  const char *s = "#Comment 1\n#Comment 2\n    # Comment 3";
  parse_t *parse = parse_init(s);
  char *ret;
  parse_skip_space(parse);
  assert(parse_peek(parse) == '\0');
  parse_free(parse);
  // Second case
  s = "#Comment 1\n #comment2 \n  ident1  #Comment 2\n    # Comment 3";
  parse = parse_init(s);
  parse_skip_space(parse);
  ret = parse_get_ident(parse);
  printf("ret = \"%s\"\n", ret);
  assert(streq(ret, "ident1") == 1);
  parse_skip_space(parse);
  assert(parse_peek(parse) == '\0');
  parse_free(parse);
  printf("Pass\n");
  return;
}

void test_parse_color() {
  printf("========== test_parse_color ==========\n");
  const char *s = "\"#AabBCC\" \"#55aa55\" \"#000000\"";
  parse_t *parse = parse_init(s);
  uint32_t ret;
  ret = parse_get_color(parse);
  printf("ret = 0x%06X\n", ret);
  assert(ret == 0xaabbcc);
  ret = parse_get_color(parse);
  printf("ret = 0x%06X\n", ret);
  assert(ret == 0x55aa55);
  ret = parse_get_color(parse);
  printf("ret = 0x%06X\n", ret);
  assert(ret == 0x000000);
  parse_free(parse);
  // Error case; Uncomment each for errors
  //parse = parse_init("\"55aa55\""); // Missing #
  //ret = parse_get_color(parse);
  //parse_free(parse);
  //parse = parse_init("\"#aabbxy\""); // Illegal digit
  //ret = parse_get_color(parse);
  //parse_free(parse);
  //parse = parse_init("\"#aabbccdd\""); // Too long
  //ret = parse_get_color(parse);
  //parse_free(parse);
  //parse = parse_init("\"#aabbc\""); // Too short
  //ret = parse_get_color(parse);
  //parse_free(parse);
  printf("Pass\n");
  return;
}

void test_parse_sort() {
  printf("========== test_parse_sort ==========\n");
  parse_t *parse = parse_init("");
  printf("Top funcs table:\n");
  for(int i = 0;i < parse_cb_top_funcs_count;i++) {
    printf("  name %s ptr 0x%p\n", parse_cb_top_funcs[i].name, parse_cb_top_funcs[i].cb);
    if(i != 0) {
      assert(strcmp(parse_cb_top_funcs[i - 1].name, parse_cb_top_funcs[i].name) < 0);
    }
  }
  printf("Top props table:\n");
  for(int i = 0;i < parse_cb_top_props_count;i++) {
    printf("  name %s code %d\n", parse_cb_top_props[i].name, parse_cb_top_props[i].prop);
    if(i != 0) {
      assert(strcmp(parse_cb_top_props[i - 1].name, parse_cb_top_props[i].name) < 0);
    }
  }
  printf("Top entities table:\n");
  for(int i = 0;i < parse_cb_top_entities_count;i++) {
    printf("  name %s code 0x%p\n", parse_cb_top_entities[i].name, parse_cb_top_entities[i].cb);
    if(i != 0) {
      assert(strcmp(parse_cb_top_entities[i - 1].name, parse_cb_top_entities[i].name) < 0);
    }
  }
  parse_free(parse);
  printf("Pass\n");
  return;
}

void test_parse_top() {
  printf("========== test_parse_top ==========\n");
  plot_t *plot = plot_init();
  plot_open(plot, "./conf/test_top.txt");
  parse_top(plot->parse, plot);
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
  test_color_init_file();
  test_hatch_init_file();
  if(valgrind_flag == 0) test_bar_type();
  if(valgrind_flag == 0) test_py();
  test_buf();
  test_vec();
  if(valgrind_flag == 0) test_plot_legend();
  if(valgrind_flag == 0) test_plot_color_test();
  test_parse_getchar();
  test_parse_until();
  test_parse_ident();
  test_parse_expect();
  test_parse_str();
  test_parse_double();
  test_parse_int();
  test_parse_skip_space();
  test_parse_color();
  test_parse_sort();
  test_parse_top();
  printf("All test passed!\n");
  return 0;
}