
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

color_scheme_t color_schemes[] = {
  COLOR_SCHEME_GEN("mixed", color_scheme_mixed),
  COLOR_SCHEME_GEN("red", color_scheme_red),
};

// This function is re-entrant
void color_str(uint32_t color, char *buf) {
  if(color & 0xFF000000) {
    error_exit("Color value 0x%08X has non-zero upper 8 bits\n", color);
  }
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

void color_scheme_print(color_scheme_t *scheme) {
  printf("Name %s count %d base 0x%p\n", scheme->name, scheme->item_count, scheme->base);
  return;
}

//* hatch_*

char hatch_scheme_default[] = {
  '-', '+', 'x', '\\', '*', 'o', 'O', '.',
};

hatch_scheme_t hatch_schemes[] = {
  HATCH_SCHEME_GEN("default", hatch_scheme_default),
};

// Returns NULL if not found
hatch_scheme_t *hatch_find_scheme(const char *name) {
  for(int i = 0;i < (int)(sizeof(hatch_schemes) / sizeof(hatch_scheme_t));i++) {
    if(streq(hatch_schemes[i].name, name) == 1) {
      return hatch_schemes + i;
    }
  }
  return NULL;
}

void hatch_scheme_print(hatch_scheme_t *scheme) {
  printf("Name %s count %d base 0x%p\n", scheme->name, scheme->item_count, scheme->base);
  return;
}

//* py_t

static int py_count = 0;

py_t *py_init() {
  py_t *py = (py_t *)malloc(sizeof(py_t));
  SYSEXPECT(py != NULL);
  memset(py, 0x00, sizeof(py_t));
  // This is recommended by Python doc
  Py_SetProgramName("matplotlib-c");
  if(py_count == 0) Py_Initialize();
  py_count++;
  return py;
}

void py_free(py_t *py) {
  // Only free when ref count is zero
  py_count--;
  if(py_count == 0) Py_Finalize();
  free(py);
  return;
}

void py_run(py_t *py, const char *s) {
  int ret = PyRun_SimpleString(s);
  if(ret != 0) {
    error_exit("Python interpreter raises an exception. Exiting.\n");
  }
  (void)py;
  return;
}

int py_get_instance_count() { return py_count; }

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

void buf_append_color(buf_t *buf, uint32_t color) {
  char s[COLOR_STRLEN + 1];
  color_str(color, s);
  buf_append(buf, s);
  return;
}

void buf_print(buf_t *buf, int content) {
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

bar_type_t *bar_type_init(const char *label) {
  bar_type_t *type = (bar_type_t *)malloc(sizeof(bar_type_t));
  SYSEXPECT(type != NULL);
  memset(type, 0x00, sizeof(bar_type_t));
  type->label = (char *)malloc(strlen(label) + 1);
  SYSEXPECT(type->label != NULL);
  strcpy(type->label, label);
  return type;
}

void bar_type_free(bar_type_t *type) {
  free(type->label);
  free(type);
  return;
}

// Duplicate the type object; Deep copy, if there is internal structure
// Except that "used" variable is set to zero
bar_type_t *bar_type_dup(bar_type_t *type) {
  // This will perform a copy of the label string
  bar_type_t *new_type = bar_type_init(type->label);
  new_type->color = type->color;
  new_type->hatch = type->hatch;
  new_type->used = 0;
  new_type->next = NULL;
  return new_type;
}

void bar_type_print(bar_type_t *type) {
  printf("[bar_type_t] label \"%s\" color 0x%08X hatch '%c' used %d\n",
         type->label, type->color, type->hatch, type->used);
  return;
}

// Bar type object is init'ed to NULL, but it must be set before being drawn
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

//* plot_t

plot_param_t default_param = {
  0,        // legend_vertical
  28,       // legend_font_size
  "best",   // Legend pos; Alternatives are: {lower, center, upper} x {left, center, right} or "center"
  24, 24,   // x/y tick font size
  28, 28,   // x/y title font size
};

// We use "plot" as the root name of the plot; "fig" as the name of the figure object
const char *plot_preamble = \
  "import sys\n"
  "import matplotlib as mpl\n"
  "import matplotlib.pyplot as plot\n"
  "import matplotlib.ticker as ticker\n"
  "\n"
  "mpl.rcParams['ps.useafm'] = True\n"
  "mpl.rcParams['pdf.use14corefonts'] = True\n"
  "mpl.rcParams['text.usetex'] = True\n"
  "mpl.rcParams['text.latex.preamble'] = [\n"
  "  r'\\usepackage{siunitx}',\n"
  "  r'\\sisetup{detect-all}',\n"
  "  r'\\usepackage{helvet}',\n"
  "  r'\\usepackage{sansmath}',\n"
  "  r'\\sansmath'\n"
  "]\n"
  "\n";

plot_t *plot_init() {
  plot_t *plot = (plot_t *)malloc(sizeof(plot_t));
  SYSEXPECT(plot != NULL);
  memset(plot, 0x00, sizeof(plot_t));
  // Initialize member variables
  plot->py = py_init();
  plot->buf = buf_init();
  buf_append(plot->buf, plot_preamble);
  // Init param
  memcpy(&plot->param, &default_param, sizeof(plot_param_t));
  return plot;
}

void plot_free(plot_t *plot) {
  buf_free(plot->buf);
  py_free(plot->py);
  // Frees type array
  bar_type_t *curr = plot->bar_types;
  while(curr) {
    bar_type_t *next = curr->next;
    bar_type_free(curr);
    curr = next;
  }
  return;
}

// Adds bar type
void plot_add_bar_type(plot_t *plot, const char *label, uint32_t color, char hatch) {
  if(plot_find_bar_type(plot, label) != NULL) {
    error_exit("The label \"%s\" already exists\n", label);
  }
  bar_type_t *type = bar_type_init(label);
  type->color = color;
  type->hatch = hatch;
  type->next = plot->bar_types;
  plot->bar_types = type;
  return;
}

// Returns NULL if not found
bar_type_t *plot_find_bar_type(plot_t *plot, const char *label) {
  bar_type_t *curr = plot->bar_types;
  while(curr) {
    if(streq(curr->label, label) == 1) {
      return curr;
    }
    curr = curr->next;
  }
  return NULL;
}

void plot_create_fig(plot_t *plot, double width, double height) {
  if(plot->fig_created == 1) {
    error_exit("A figure has already been created on this plot\n");
  }
  buf_printf(plot->buf, "fig = plot.figure(figsize=(%f, %f))\n", width, height);
  // "111" means the output consists of only one plot
  buf_append(plot->buf, "ax = fig.add_subplot(111)\n\n");
  plot->fig_created = 1;
  return;
}

void plot_save_fig(plot_t *plot, const char *filename) {
  if(plot->fig_created == 0) {
    error_exit("The figure has not been created yet\n");
  }
  buf_printf(plot->buf, "plot.savefig(\"%s\", bbox_inches='tight')\n\n", filename);
  py_run(plot->py, buf_c_str(plot->buf));
  return;
}

// Saves a standalone legend file
// This function can be called anywhere during the plotting procedure. Legends drawn will be 
// bar types stored in the plot object
void plot_save_legend(plot_t *plot, const char *filename) {
  plot_t *legend = plot_init(); // Preamble is set after this
  assert(legend->buf != NULL && legend->py != NULL);
  plot_create_fig(legend, 0.001f, 0.001f);
  bar_type_t *curr = plot->bar_types;
  if(curr == NULL) {
    error_exit("Current plot does not contain any bar type\n");
  }
  while(curr) {
    bar_t *bar = bar_init();
    // The bar should not be drawn
    bar->bottom = bar->height = bar->width = 0.0;
    // Also duplicate the bar type and associate it with the bar
    plot_add_bar_type(legend, curr->label, curr->color, curr->hatch);
    bar_set_type(bar, plot_find_bar_type(legend, curr->label));
    plot_add_bar(legend, bar);
    // No longer used
    bar_free(bar);
    curr = curr->next;
  }
  legend->param.legend_pos = "center"; // Hardcode lagend pos
  plot_add_legend(legend);
  plot_save_fig(legend, filename);
  plot_free(legend);
  return;
}

// Only one new line is appended at the end of the draw
void plot_add_bar(plot_t *plot, bar_t *bar) {
  assert(bar->type != NULL);
  buf_t *buf = plot->buf;
  // Firts two args are fixed
  buf_printf(buf, "ax.bar(%f, %f\n", bar->pos, bar->height);
  // Following args are optional
  buf_printf(buf, "  , width=%f\n", bar->width);
  if(bar->bottom != 0.0) buf_printf(buf, "  , bottom=%f\n", bar->bottom);
  // Color of the bar
  buf_printf(buf, "  , color='");
  buf_append_color(buf, bar_get_color(bar));
  buf_printf(buf, "'\n");
  // Hatch (if not '\0')
  char hatch = bar_get_hatch(bar);
  if(hatch != '\0') {
    if(hatch == '\\') buf_printf(buf, "  , hatch='\\\\'\n");
    else buf_printf(buf, "  , hatch='%c'\n", hatch);
  }
  // Add label if it has not been used for bars
  if(bar_get_type(bar)->used == 0) {
    bar_get_type(bar)->used = 1;
    buf_printf(buf, "  , label='%s'\n", bar_get_type(bar)->label);
  }
  // This concludes arg list of bar()
  buf_printf(buf, ")\n");
  return;
}

// Uses legend font size, legend vertical, and legend position in the param object
void plot_add_legend(plot_t *plot) {
  int col_count = 0;
  // Compute col_count by counting bar types
  if(plot->param.legend_vertical == 1) {
    col_count = 1;
  } else {
    bar_type_t *curr = plot->bar_types;
    if(curr == NULL) {
      error_exit("Current plot does not contain any bar type\n");
    }
    while(curr) {
      col_count++;
      curr = curr->next;
    }
  }
  buf_t *buf = plot->buf;
  plot_param_t *param = &plot->param;
  // Adding legend statement
  buf_printf(buf, "ax.legend(loc=\"%s\", prop={'size':%d}, ncol=%d)\n\n",
             param->legend_pos, param->legend_font_size, col_count);
  return;
}

void plot_print(plot_t *plot) {
  buf_print(plot->buf);
  return;
}