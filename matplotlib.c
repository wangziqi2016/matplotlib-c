
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

uint32_t color_scheme_blue[] = {
  COLOR_GEN(0x0a, 0x11, 0x72),
  COLOR_GEN(0x13, 0x38, 0xbe),
  COLOR_GEN(0x04, 0x92, 0xc2),
  COLOR_GEN(0x63, 0xc5, 0xda),
  COLOR_GEN(0x82, 0xee, 0xfd),
};

uint32_t color_scheme_grey[] = {
  COLOR_GEN(0xee, 0xee, 0xee),
  COLOR_GEN(0xcc, 0xcc, 0xcc),
  COLOR_GEN(0xaa, 0xaa, 0xaa),
  COLOR_GEN(0x88, 0x88, 0x88),
  COLOR_GEN(0x66, 0x66, 0x66),
  COLOR_GEN(0x44, 0x44, 0x44),
  COLOR_GEN(0x22, 0x22, 0x22),
  COLOR_GEN(0x00, 0x00, 0x00),
};

color_scheme_t color_schemes[] = {
  COLOR_SCHEME_GEN("mixed", color_scheme_mixed),
  COLOR_SCHEME_GEN("red", color_scheme_red),
  COLOR_SCHEME_GEN("blue", color_scheme_blue),
  COLOR_SCHEME_GEN("grey", color_scheme_grey),
};

color_scheme_t *color_scheme_init(const char *name, uint32_t *base, int item_count) {
  color_scheme_t *scheme = (color_scheme_t *)malloc(sizeof(color_scheme_t));
  SYSEXPECT(scheme != NULL);
  memset(scheme, 0x00, sizeof(color_scheme_t));
  int len = strlen(name);
  scheme->name = (char *)malloc(len + 1);
  SYSEXPECT(scheme->name != NULL);
  strcpy(scheme->name, name);
  scheme->base = (uint32_t *)malloc(COLOR_SIZE * item_count);
  SYSEXPECT(scheme->base != NULL);
  memcpy(scheme->base, base, COLOR_SIZE * item_count);
  scheme->item_count = item_count;
  return scheme;
}

// This function does not terminate program on error; It returns NULL and caller should handle error
color_scheme_t *color_scheme_init_file(const char *filename) {
  FILE *fp = fopen(filename, "r");
  if(fp == NULL) {
    printf("Could not open file \"%s\" for read\n", filename);
    return NULL;
  }
  // We use the file name as scheme name
  // Use item_count as capacity when reading from file
  color_scheme_t *scheme = (color_scheme_t *)malloc(sizeof(color_scheme_t));
  SYSEXPECT(scheme != NULL);
  memset(scheme, 0x00, sizeof(color_scheme_t));
  int len = strlen(filename);
  scheme->name = (char *)malloc(len + 1);
  SYSEXPECT(scheme->name != NULL);
  strcpy(scheme->name, filename);
  scheme->base = (uint32_t *)malloc(COLOR_SIZE * COLOR_INIT_FILE_COUNT);
  SYSEXPECT(scheme->base != NULL);
  scheme->item_count = COLOR_INIT_FILE_COUNT;
  int count = 0; // item count
  int line = 1;  // For error reporting
  char buf[16];
  while(fgets(buf, 16, fp) != NULL) {
    // Jump over space
    char *p = buf;
    while(isspace(*p)) p++;
    int len = strlen(p);
    if(len == 0) {
      continue;
    } if(len < 7) {
      printf("Illegal color in line %d: \"%s\"\n", line, buf);
      return NULL;
    }
    // Check if any non-space char after the color code
    char *q = p + 7;
    while(isspace(*q)) q++;
    if(*q != '\0') {
      printf("Illegal color in line %d: \"%s\"\n", line, buf);
      return NULL;
    }
    // Length check passed, truncate first 7 non-space chars
    p[7] = '\0';
    // This may also print error message
    uint32_t color = color_decode(p);
    if(color == -1U) return NULL;
    assert(count <= scheme->item_count);
    // Expand the base array if it is too small
    if(count == scheme->item_count) {
      uint32_t *old = scheme->base;
      scheme->item_count *= 2;
      scheme->base = (uint32_t *)malloc(COLOR_SIZE * scheme->item_count);
      SYSEXPECT(scheme->base != NULL);
      memcpy(scheme->base, old, COLOR_SIZE * (scheme->item_count / 2));
      free(old);
    }
    assert(count < scheme->item_count);
    scheme->base[count++] = color;
    line++;
  }
  fclose(fp);
  // Now this represents item count, not capacity
  scheme->item_count = count;
  return scheme;
}

void color_scheme_free(color_scheme_t *scheme) {
  free(scheme->name);
  free(scheme->base);
  free(scheme);
  return;
}

// This function is re-entrant
// Users should call color_str() / color_str_latex()
void _color_str(uint32_t color, char *buf, int for_latex) {
  if(color & 0xFF000000) {
    error_exit("Color value 0x%08X has non-zero upper 8 bits\n", color);
  }
  if(for_latex == 1) {
    sprintf(buf, "\\#%02X%02X%02X", COLOR_R(color), COLOR_G(color), COLOR_B(color));
  } else {
    sprintf(buf, "#%02X%02X%02X", COLOR_R(color), COLOR_G(color), COLOR_B(color));
  }
  return;
}

// Returns 0xFFFFFFFF to indicate failure
// This function does not report error; Instead, it prints error message, and caller should handle it
uint32_t color_decode(const char *s) {
  if(*s++ != '#') {
    printf("Color code must begin with '#' (see \"%s\")\n", s);
    return -1u;
  }
  uint32_t ret = 0u;
  int shift = 20;
  for(int i = 0;i < 6;i++) {
    char ch = *s++;
    if(ch == '\0') {
      printf("Unexpected end of color code\n");
      return -1u;
    }
    uint32_t hex;
    if((ch >= '0' && ch <= '9')) {
      hex = (uint32_t)(ch - '0');
    } else if(ch >= 'a' && ch <= 'f') {
      hex = (uint32_t)(ch - 'a' + 10);
    } else if(ch >= 'A' && ch <= 'F') {
      hex = (uint32_t)(ch - 'A' + 10);
    } else {
      const char *rgb;
      if(i <= 1) rgb = "Red";
      else if(i <= 3) rgb = "Green";
      else rgb = "Blue";
      printf("Invalid color code: \"%s\"; Component \"%s\" contains invalid digit\n", s, rgb);
      return -1u;
    }
    assert((hex & ~0xffu) == 0);
    ret |= (hex << shift);
    shift -= 4;
  }
  if(*s != '\0') {
    printf("Unexpected character after color code\n");
    return -1u;
  }
  return ret;
}

// Duplicates an existing scheme
color_scheme_t *color_scheme_dup(color_scheme_t *scheme) {
  return color_scheme_init(scheme->name, scheme->base, scheme->item_count);
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

void color_scheme_print(color_scheme_t *scheme, int print_content) {
  printf("[color] Name \"%s\" count %d base 0x%p\n", scheme->name, scheme->item_count, scheme->base);
  if(print_content == 1) {
    for(int i = 0;i < scheme->item_count;i++) {
      char buf[16];
      color_str(scheme->base[i], buf);
      printf("  Index %d color %s\n", i, buf);
    }
  }
  return;
}

//* hatch_*

hatch_scheme_t *hatch_scheme_init(const char *name, char *base, int item_count) {
  hatch_scheme_t *scheme = (hatch_scheme_t *)malloc(sizeof(hatch_scheme_t));
  SYSEXPECT(scheme != NULL);
  memset(scheme, 0x00, sizeof(hatch_scheme_t));
  int len = strlen(name);
  scheme->name = (char *)malloc(len + 1);
  SYSEXPECT(scheme->name != NULL);
  strcpy(scheme->name, name);
  scheme->base = (char *)malloc(HATCH_SIZE * item_count);
  SYSEXPECT(scheme->base != NULL);
  memcpy(scheme->base, base, HATCH_SIZE * item_count);
  scheme->item_count = item_count;
  return scheme;
}

hatch_scheme_t *hatch_scheme_init_file(const char *filename) {
  FILE *fp = fopen(filename, "r");
  if(fp == NULL) {
    printf("Could not open file \"%s\" for read\n", filename);
    return NULL;
  }
  hatch_scheme_t *scheme = (hatch_scheme_t *)malloc(sizeof(hatch_scheme_t));
  SYSEXPECT(scheme != NULL);
  memset(scheme, 0x00, sizeof(hatch_scheme_t));
  int len = strlen(filename);
  scheme->name = (char *)malloc(len + 1);
  SYSEXPECT(scheme->name != NULL);
  strcpy(scheme->name, filename);
  scheme->base = (char *)malloc(HATCH_SIZE * HATCH_INIT_FILE_COUNT);
  SYSEXPECT(scheme->base != NULL);
  scheme->item_count = HATCH_INIT_FILE_COUNT;
  int count = 0; // item count
  int line = 1;  // For error reporting
  char buf[16];
  while(fgets(buf, 16, fp) != NULL) {
    // Jump over space
    char *p = buf;
    while(isspace(*p)) p++;
    int len = strlen(p);
    if(len == 0) continue;
    // Removing trailing spaces
    char *q = p + len - 1;
    while(isspace(*q)) { *q = '\0'; q--; }
    len = strlen(p);
    //printf("len %d p[1] %d valid %d\n", len, len == 2 ? p[1] : -1, hatch_is_valid(p[0]));
    // Length either 1 or 2; If it is 2 then p[1] must be \n
    if(len > 2 || (len == 2 && p[1] != '\n') || (hatch_is_valid(p[0]) == 0)) {
      printf("Illegal hatch in line %d: \"%s\"\n", line, buf);
      return NULL;
    }
    assert(count <= scheme->item_count);
    // Expand the base array if it is too small
    if(count == scheme->item_count) {
      char *old = scheme->base;
      scheme->item_count *= 2;
      scheme->base = (char *)malloc(HATCH_SIZE * scheme->item_count);
      SYSEXPECT(scheme->base != NULL);
      memcpy(scheme->base, old, HATCH_SIZE * (scheme->item_count / 2));
      free(old);
    }
    assert(count < scheme->item_count);
    scheme->base[count++] = p[0];
    line++;
  }
  fclose(fp);
  // Now this represents item count, not capacity
  scheme->item_count = count;
  return scheme;
}

void hatch_scheme_free(hatch_scheme_t *scheme) {
  free(scheme->name);
  free(scheme->base);
  free(scheme);
  return;
}

// This is a full list of charracter hatches supported by matplotlib
char hatch_scheme_all[] = {
  '-', '+', 'x', '\\', '*', 'o', 'O', '.', ',', 'v', '^', '<', '>', '1', '2', '3', '4', '8',
  's', 'p', 'P', 'h', 'H', 'X', 'd', 'D', '|', '_', '/',
};
int hatch_scheme_all_count = sizeof(hatch_scheme_all) / sizeof(char); 

char *hatch_scheme_default = hatch_scheme_all;

hatch_scheme_t hatch_schemes[] = {
  HATCH_SCHEME_GEN("default", hatch_scheme_all),
};

int hatch_is_valid(char hatch) {
  for(int i = 0;i < hatch_scheme_all_count;i++) {
    if(hatch_scheme_all[i] == hatch) return 1;
  }
  return 0;
}

// Returns NULL if not found
hatch_scheme_t *hatch_find_scheme(const char *name) {
  for(int i = 0;i < (int)(sizeof(hatch_schemes) / sizeof(hatch_scheme_t));i++) {
    if(streq(hatch_schemes[i].name, name) == 1) {
      return hatch_schemes + i;
    }
  }
  return NULL;
}

hatch_scheme_t *hatch_scheme_dup(hatch_scheme_t *scheme) {
  return hatch_scheme_init(scheme->name, scheme->base, scheme->item_count);
}

void hatch_scheme_print(hatch_scheme_t *scheme, int print_content) {
  printf("[hatch] Name \"%s\" count %d base 0x%p\n", scheme->name, scheme->item_count, scheme->base);
  if(print_content == 1) {
    for(int i = 0;i < scheme->item_count;i++) {
      printf("  Index %d hatch '%c'\n", i, scheme->base[i]);
    }
  }
  return;
}

//* py_t

static int py_count = 0;
static int inited = 0;

py_t *py_init() {
  py_t *py = (py_t *)malloc(sizeof(py_t));
  SYSEXPECT(py != NULL);
  memset(py, 0x00, sizeof(py_t));
  // This is recommended by Python doc
  Py_SetProgramName("matplotlib-c");
  // Always only init/fin python lib once during the program's lifetime
  // Some modules do not clean up properly and will crash otherwise
  if(inited == 0) {
    inited = 1;
    Py_Initialize();
    atexit(Py_Finalize);
  }
  //if(py_count == 0) Py_Initialize();
  py_count++;
  return py;
}

void py_free(py_t *py) {
  // Only free when ref count is zero
  assert(py_count != 0);
  py_count--;
  // Don't init/fin multiple times!
  // This will cause numpy to crash
  //if(py_count == 0) Py_Finalize();
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

//* vec_t

vec_t *vec_init() {
  vec_t *vec = (vec_t *)malloc(sizeof(vec_t));
  SYSEXPECT(vec != NULL);
  memset(vec, 0x00, sizeof(vec_t));
  vec->count = 0;
  vec->capacity = VEC_INIT_COUNT;
  vec->data = (void **)malloc(sizeof(void *) * VEC_INIT_COUNT);
  SYSEXPECT(vec->data != NULL);
  return vec;
}

void vec_free(vec_t *vec) {
  free(vec->data);
  free(vec);
  return;
}

void vec_append(vec_t *vec, void *p) {
  assert(vec->count >= 0 && vec->count <= vec->capacity);
  if(vec->count == vec->capacity) {
    vec->capacity *= 2;
    void **old = vec->data;
    vec->data = (void **)malloc(sizeof(void *) * vec->capacity);
    SYSEXPECT(vec->data != NULL);
    memcpy(vec->data, old, sizeof(void *) * vec->count);
    free(old);
  }
  assert(vec->count < vec->capacity);
  vec->data[vec->count++] = p;
  return;
}

void vec_print(vec_t *vec) {
  printf("[vec_t] count %d cap %d data %p\n", vec->count, vec->capacity, vec->data);
  return;
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

void buf_putchar(buf_t *buf, char ch) {
  buf_realloc(buf, buf->size + 1); 
  // Before this, buf[size - 1] is '\0'
  buf->data[buf->size - 1] = ch;
  buf->data[buf->size] = '\0';
  buf->size++;
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
  printf("[buf_t] size %d cap %d data 0x%p\n", buf->size, buf->capacity, buf->data);
  if(content == 1) {
    printf("---------- contents ----------\n");
    printf("%s\n", buf->data);
    printf("------------ end -------------\n");
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

// Copies the text into a separate buffer and frees the previous one, if any
void bar_set_text(bar_t *bar, const char *text) {
  if(bar->text != NULL) free(bar->text);
  bar->text = (char *)malloc(strlen(text) + 1);
  SYSEXPECT(bar->text != NULL);
  strcpy(bar->text, text);
  return;
}

void bar_print(bar_t *bar) {
  printf("[bar_t] height %f width %f pos %f bottom %f text \"%s\" label \"%s\"\n",
    bar->height, bar->width, bar->pos, bar->bottom, bar->text,
    bar->type ? bar->type->label : "");
  return;
}

//* plot_t

plot_param_t default_param = {
  12.0, 6.0, // Width and Height
  1,         // legend_rows
  28,        // legend_font_size
  "best",    // Legend pos; Alternatives are: {lower, center, upper} x {left, center, right} or "center"
  24, 0,     // x tick font size, rotation
  24, 0,     // y tick font size, rotation
  28, 28,    // x/y title font size
  26, 90,    // bar text size, rotation
  2, 1,      // bar text decimals, rtrim
  NULL, 0,   // Hatch scheme/offset
  NULL, 0,   // Color scheme/offset
  INFINITY, INFINITY, // xlimits
  INFINITY, INFINITY, // ylimits
  0,         // Dry run
};

void plot_param_copy(plot_param_t *dst, plot_param_t *src) {
  if(dst->color_scheme != NULL) color_scheme_free(dst->color_scheme);
  if(dst->hatch_scheme != NULL) hatch_scheme_free(dst->hatch_scheme);
  memcpy(dst, src, sizeof(plot_param_t));
  if(src->color_scheme != NULL) dst->color_scheme = color_scheme_dup(src->color_scheme);
  if(src->hatch_scheme != NULL) dst->hatch_scheme = hatch_scheme_dup(src->hatch_scheme);
  return;
}

void plot_param_print(plot_param_t *param, int verbose) {
  printf("[param] width %f height %f\n", param->width, param->height);
  printf("[param legend] font size %d rows %d pos \"%s\"\n", 
    param->legend_font_size, param->legend_rows, param->legend_pos);
  printf("[param title] x font %d y font %d\n", 
    param->xtitle_font_size, param->ytitle_font_size);
  printf("[param xtick] font %d rot %d\n", 
    param->xtick_font_size, param->xtick_rotation);
  printf("[param ytick] font %d rot %d\n", 
    param->ytick_font_size, param->ytick_rotation);
  printf("[param bar_text] font %d rotation %d\n", param->bar_text_font_size, param->bar_text_rotation);
  printf("[param bar_text] decimals %d rtrim %d\n", 
    param->bar_text_decimals, param->bar_text_rtrim);
  if(param->hatch_scheme != NULL) {
    // There is no nwe line after this, if verbose is turned on
    printf("[param hatch] Offset %d (usable %d) ", 
      param->hatch_offset, param->hatch_scheme->item_count - param->hatch_offset);
    if(verbose == 1) hatch_scheme_print(param->hatch_scheme, verbose);
    else putchar('\n');
  }
  if(param->color_scheme != NULL) {
    printf("[param color] Offset %d (usable %d) ",  
      param->color_offset, param->color_scheme->item_count - param->color_offset);
    if(verbose == 1) color_scheme_print(param->color_scheme, verbose);
    else putchar('\n');
  }
  printf("[param x/y_lim] left %f right %f top %f bottom %f\n",
    param->xlim_left, param->xlim_right, param->ylim_top, param->ylim_bottom);
  return;
}

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
  "\n"
  // The following are used to draw x/y ticks
  "cmatplotlib_xticks = []\n"
  "cmatplotlib_xtick_labels = []\n"
  "cmatplotlib_yticks = []\n"
  "cmatplotlib_ytick_labels = []\n\n";

plot_t *plot_init() {
  plot_t *plot = (plot_t *)malloc(sizeof(plot_t));
  SYSEXPECT(plot != NULL);
  memset(plot, 0x00, sizeof(plot_t));
  // Initialize member variables
  plot->py = py_init();
  plot->buf = buf_init();
  plot->bar_types = vec_init();
  buf_append(plot->buf, plot_preamble);
  // Init param
  memcpy(&plot->param, &default_param, sizeof(plot_param_t));
  return plot;
}

void plot_free(plot_t *plot) {
  buf_free(plot->buf);
  py_free(plot->py);
  // Frees type array
  for(int i = 0;i < vec_count(plot->bar_types);i++) {
    bar_type_free((bar_type_t *)vec_at(plot->bar_types, i));
  }
  vec_free(plot->bar_types);
  // Note that parse can be NULL if it is not initialized
  if(plot->parse != NULL) parse_free(plot->parse);
  if(plot->xtitle) free(plot->xtitle);
  if(plot->ytitle) free(plot->ytitle);
  if(plot->fig_filename) free(plot->fig_filename);
  if(plot->legend_filename) free(plot->legend_filename);
  plot_param_t *param = &plot->param;
  // Free color and hatch scheme if they are allocated
  if(param->color_scheme != NULL) color_scheme_free(param->color_scheme);
  if(param->hatch_scheme != NULL) hatch_scheme_free(param->hatch_scheme);
  return;
}

// Open a script file for parsing
void plot_open(plot_t *plot, const char *filename) {
  plot->parse = parse_init_file(filename);
  return;
}

// The string will be copied into the parser
void plot_open_str(plot_t *plot, const char *s) {
  plot->parse = parse_init(s);
  return;
}

// Adds bar type; Note that types preserve the order they are inserted
void plot_add_bar_type(plot_t *plot, const char *label, uint32_t color, char hatch) {
  if(plot_find_bar_type(plot, label) != NULL) {
    error_exit("The label \"%s\" already exists\n", label);
  }
  bar_type_t *type = bar_type_init(label);
  type->color = color;
  type->hatch = hatch;
  vec_append(plot->bar_types, type);
  return;
}

// Returns NULL if not found
bar_type_t *plot_find_bar_type(plot_t *plot, const char *label) {
  for(int i = 0;i < vec_count(plot->bar_types);i++) {
    bar_type_t *type = (bar_type_t *)vec_at(plot->bar_types, i);
    if(streq(type->label, label) == 1) {
      return type;
    }
  }
  return NULL;
}

// This function uses param object's width and height
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
  buf_t *buf = plot->buf;
  plot_param_t *param = &plot->param;
  // Set x tick and y tick
  buf_printf(buf, "if len(cmatplotlib_xticks) != 0:\n");
  buf_printf(buf, "  plot.xticks(cmatplotlib_xticks, cmatplotlib_xtick_labels");
  buf_printf(buf, ", fontsize=%d", param->xtick_font_size);
  if(param->xtick_rotation != 0) {
    //printf("rotation %d\n", param->xtick_rotation);
    buf_printf(buf, ", rotation=%d", param->xtick_rotation);
  }
  buf_printf(buf, ")\n");
  buf_printf(buf, "if len(cmatplotlib_yticks) != 0:\n");
  buf_printf(buf, "  plot.yticks(cmatplotlib_yticks, cmatplotlib_ytick_labels");
  buf_printf(buf, ", fontsize=%d", param->ytick_font_size);
  if(param->xtick_rotation != 0) {
    buf_printf(buf, ", rotation=%d", param->ytick_rotation);
  }
  buf_printf(buf, ")\n");
  // Set X/Y limits
  if(param->xlim_left != INFINITY) buf_printf(buf, "ax.set_xlim(left=%f)\n", param->xlim_left);
  if(param->xlim_right != INFINITY) buf_printf(buf, "ax.set_xlim(right=%f)\n", param->xlim_right);
  if(param->ylim_top != INFINITY) buf_printf(buf, "ax.set_ylim(top=%f)\n", param->ylim_top);
  if(param->ylim_bottom != INFINITY) buf_printf(buf, "ax.set_ylim(bottom=%f)\n", param->ylim_bottom);
  // Pass the file name
  buf_printf(plot->buf, "plot.savefig(\"%s\", bbox_inches='tight')\n\n", filename);
  // Execute script
  if(param->dry_run == 0) {
    py_run(plot->py, buf_c_str(plot->buf));
  } else {
    printf("[plot] Dry run mode is on; not executing anything\n");
  }
  return;
}

// Saves a standalone legend file
// This function can be called anywhere during the plotting procedure. Legends drawn will be 
// bar types stored in the plot object
void plot_save_legend(plot_t *plot, const char *filename) {
  plot_t *legend = plot_init(); // Preamble is set after this
  plot_param_copy(&legend->param, &plot->param); // Copy legend configuration to the new legend plot
  legend->param.width = legend->param.height = 0.001; // Super small graph
  assert(legend->buf != NULL && legend->py != NULL);
  plot_create_fig(legend, legend->param.width, legend->param.height);
  int count = vec_count(plot->bar_types);
  if(count == 0) {
    error_exit("Current plot does not contain any bar type\n");
  }
  for(int i = 0;i < count;i++) {
    bar_type_t *type = vec_at(plot->bar_types, i);
    bar_t *bar = bar_init();
    // The bar should not be drawn
    bar->bottom = bar->height = bar->width = 0.0;
    // Also duplicate the bar type and associate it with the bar
    plot_add_bar_type(legend, type->label, type->color, type->hatch);
    bar_set_type(bar, plot_find_bar_type(legend, type->label));
    plot_add_bar(legend, bar);
    // No longer used
    bar_free(bar);
  }
  plot_set_legend_pos(legend, "center");  // Hardcode lagend pos
  plot_add_legend(legend);
  plot_save_fig(legend, filename);
  plot_free(legend);
  return;
}

// This functions draws a color test bar graph
void plot_save_color_test(plot_t *plot, const char *filename) {
  plot_t *test = plot_init();
  // Use current plot's configuration
  plot_param_copy(&test->param, &plot->param);
  plot_create_fig(test, test->param.width, test->param.height);
  plot_param_t *param = &test->param;
  char label_buf[16];
  int usable = param->color_scheme->item_count - param->color_offset;
  double bar_width = param->width / (double)usable;
  double bar_height = param->height;
  double bar_pos = 0.0;
  for(int i = param->color_offset;i < param->color_scheme->item_count;i++) {
    snprintf(label_buf, 16, "color %d", i);
    plot_add_bar_type(test, label_buf, param->color_scheme->base[i], '\0');
    bar_t *bar = bar_init();
    bar->pos = bar_pos;
    bar->width = bar_width;
    bar->height = bar_height;
    bar_set_type(bar, plot_find_bar_type(test, label_buf));
    // Print color code
    char color_buf[16];
    color_str_latex(bar_get_type(bar)->color, color_buf);
    bar_set_text(bar, color_buf);
    plot_add_bar(test, bar);
    char xtick_text[16];
    snprintf(xtick_text, 16, "[%d]", i);
    plot_add_xtick(test, bar_pos + 0.5 * bar_width, xtick_text);
    bar_free(bar);
    bar_pos += bar_width;
  }
  plot_add_x_title(test, "Color Scheme Test");
  plot_save_fig(test, filename);
  plot_free(test);
  return;
}

void plot_save_hatch_test(plot_t *plot, const char *filename) {
  plot_t *test = plot_init();
  // Use current plot's configuration
  plot_param_copy(&test->param, &plot->param);
  plot_param_t *param = &test->param;
  char label_buf[16];
  int usable = param->hatch_scheme->item_count - param->color_offset;
  double bar_width = 2.0; // To show the hatch we need fixed width bar
  param->width = usable * bar_width; // Graph width is extended as there are more hatches
  param->xlim_right = usable * bar_width; // Set X right limit to avoid blank
  double bar_height = param->height;
  double bar_pos = 0.0;
  // Must do it here since we adjusted the width
  plot_create_fig(test, param->width, param->height);
  for(int i = param->color_offset;i < param->hatch_scheme->item_count;i++) {
    snprintf(label_buf, 16, "hatch %d", i);
    char hatch = param->hatch_scheme->base[i];
    plot_add_bar_type(test, label_buf, 0xFFFFFF, hatch);
    bar_t *bar = bar_init();
    bar->pos = bar_pos;
    bar->width = bar_width;
    bar->height = bar_height;
    bar_set_type(bar, plot_find_bar_type(test, label_buf));
    // Print color code
    char hatch_buf[32];
    // In python this will be "\\..." and actual binary seen by latex is "\..."
    if(hatch == '\\') snprintf(hatch_buf, sizeof(hatch_buf), "\\\\textbackslash");
    else if(hatch == '^') snprintf(hatch_buf, sizeof(hatch_buf), "\\\\textasciicircum"); 
    else if(hatch == '_') snprintf(hatch_buf, sizeof(hatch_buf), "\\\\_"); 
    else if(hatch == '%') snprintf(hatch_buf, sizeof(hatch_buf), "\\\\%%"); 
    else if(hatch == '$') snprintf(hatch_buf, sizeof(hatch_buf), "\\\\$"); 
    else snprintf(hatch_buf, 16, "%c", hatch);
    bar_set_text(bar, hatch_buf);
    plot_add_bar(test, bar);
    char xtick_text[16];
    snprintf(xtick_text, 16, "[%d]", i);
    plot_add_xtick(test, bar_pos + 0.5 * bar_width, xtick_text);
    bar_free(bar);
    bar_pos += bar_width;
  }
  plot_add_x_title(test, "Hatch Scheme Test");
  plot_save_fig(test, filename);
  plot_free(test);
  return;
}

// Sets legend pos by copying the string to the param struct
// The given string should not be longer than the storage size
void plot_set_legend_pos(plot_t *plot, const char *pos) {
  int len = strlen(pos);
  if(len > PLOT_LEGEND_POS_MAX_SIZE - 1) {
    error_exit("Legend pos must be a string shorter than %d bytes (sees %d)\n", 
      PLOT_LEGEND_POS_MAX_SIZE, len);
  }
  strcpy(plot->param.legend_pos, pos);
  return;
}

void plot_set_legend_rows(plot_t *plot, int rows) {
  if(rows <= 0 && rows != -1) {
    error_exit("Legend rows must be > 0 or -1 for vertical legend (sees %d)\n", rows);
  }
  plot->param.legend_rows = rows;
  return;
}

// Only one new line is appended at the end of the draw
void plot_add_bar(plot_t *plot, bar_t *bar) {
  assert(bar->type != NULL);
  buf_t *buf = plot->buf;
  plot_param_t *param = &plot->param;
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
    else if(hatch == '%') buf_printf(buf, "  , hatch='\\\\%'\n"); // Python sees \\% latex sees \%
    else if(hatch == '$') buf_printf(buf, "  , hatch='\\\\$'\n"); // Python sees \\% latex sees \$
    else buf_printf(buf, "  , hatch='%c'\n", hatch);
  }
  // Add label if it has not been used for bars
  if(bar_get_type(bar)->used == 0) {
    bar_get_type(bar)->used = 1;
    buf_printf(buf, "  , label='%s'\n", bar_get_type(bar)->label);
  }
  // This concludes arg list of bar()
  buf_printf(buf, ")\n");
  // Then draw bar text
  char *bar_text = bar->text;
  int bar_text_free = 0;
  if(bar_text == NULL) {
    // Round the height
    bar_text = fp_print(bar->height, param->bar_text_decimals);
    bar_text_free = 1;
    if(param->bar_text_rtrim == 1) {
      fp_rtrim(bar_text);
    }
  }
  // TODO: ADJUST FOR ERROR BAR
  buf_printf(buf, "ax.text(%f, %f, '%s'", bar->pos + bar->width / 2.0, bar->height, bar_text);
  buf_printf(buf, "  , ha='center', va='bottom', rotation=%d, fontsize=%d)\n",
    param->bar_text_rotation, param->bar_text_font_size);
  if(bar_text_free == 1) free(bar_text);
  return;
}

// Pos is always aligned to the center of the text
void plot_add_xtick(plot_t *plot, double pos, const char *text) {
  buf_t *buf = plot->buf;
  buf_printf(buf, "cmatplotlib_xticks.append(%f)\n", pos);
  buf_printf(buf, "cmatplotlib_xtick_labels.append('%s')\n", text);
  return;
}

// Pos is always aligned to the center of the text
void plot_add_ytick(plot_t *plot, double pos, const char *text) {
  buf_t *buf = plot->buf;
  buf_printf(buf, "cmatplotlib_yticks.append(%f)\n", pos);
  buf_printf(buf, "cmatplotlib_ytick_labels.append('%s')\n", text);
  return;
}

// Uses legend font size, legend vertical, and legend position in the param object
void plot_add_legend(plot_t *plot) {
  int col_count = 0;
  // Total number of elements in the legend
  int type_count = vec_count(plot->bar_types);
  if(type_count == 0) {
    error_exit("Current plot does not contain any bar type\n");
  }
  int legend_rows = plot->param.legend_rows;
  assert(legend_rows > 0 || legend_rows == -1);
  if(legend_rows == -1) { // Special case: -1 means vertical
    col_count = type_count;
  } else {
    if(type_count % legend_rows == 0) {
      col_count = type_count / legend_rows;
    } else {
      col_count = (type_count + (legend_rows - type_count % legend_rows)) / legend_rows;
    }
  }
  buf_t *buf = plot->buf;
  plot_param_t *param = &plot->param;
  // Adding legend statement
  buf_printf(buf, "ax.legend(loc=\"%s\", prop={'size':%d}, ncol=%d)\n\n",
             param->legend_pos, param->legend_font_size, col_count);
  return;
}

// Adding X axis title
void plot_add_x_title(plot_t *plot, const char *title) {
  buf_printf(plot->buf, "ax.set_xlabel(\"%s\", fontsize=%lu, weight='bold')\n",
             title, plot->param.xtitle_font_size);
  return;
}

void plot_add_y_title(plot_t *plot, const char *title) {
  buf_printf(plot->buf, "ax.set_ylabel(\"%s\", fontsize=%lu, weight='bold')\n",
             title, plot->param.ytitle_font_size);
  return;
}

void plot_print(plot_t *plot, int verbose) {
  // Print plot properties
  if(plot->xtitle != NULL) printf("[plot] xtitle %s\n", plot->xtitle);
  if(plot->ytitle != NULL) printf("[plot] ytitle %s\n", plot->ytitle);
  if(plot->fig_filename != NULL) printf("[plot] fig_filename %s\n", plot->fig_filename);
  if(plot->legend_filename != NULL) printf("[plot] legend_filename %s\n", plot->legend_filename);
  // Print param
  plot_param_print(&plot->param, verbose);
  // Print bar types
  printf("[plot] bar types %d\n", vec_count(plot->bar_types));
  if(verbose == 1) {
    for(int i = 0;i < vec_count(plot->bar_types);i++) {
      bar_type_t *type = (bar_type_t *)vec_at(plot->bar_types, i);
      bar_type_print(type);
    }
  }
  // Optionally print buffer content
  buf_print(plot->buf, verbose);
  return;
}

//* parse_*

parse_t *_parse_init(char *s) {
  parse_t *parse = (parse_t *)malloc(sizeof(parse_t));
  SYSEXPECT(parse != NULL);
  memset(parse, 0x00, sizeof(parse_t));
  parse->s = parse->curr = s;
  parse->line = 1; // Line and col starts from 1
  parse->col = 0;
  parse->size = strlen(s) + 1;  // Including terminating zero
  // Sort top-level function table
  parse_sort_cb(parse, parse_cb_top_funcs, parse_cb_top_funcs_count);
  // Sort top-level properties
  parse_sort_cb(parse, parse_cb_top_props, parse_cb_top_props_count);
  // Sort top-level entities
  parse_sort_cb(parse, parse_cb_top_entities, parse_cb_top_entities_count);
  return parse;
}

parse_t *parse_init(const char *s) {
  int len = strlen(s);
  char *buf = (char *)malloc(len + 1);
  SYSEXPECT(buf != NULL);
  memcpy(buf, s, len + 1);
  parse_t *parse = _parse_init(buf);
  char filename[] = "[debug]";
  parse->filename = (char *)malloc(strlen(filename) + 1);
  SYSEXPECT(parse->filename != NULL);
  strcpy(parse->filename, filename);
  return parse;
}

parse_t *parse_init_file(const char *filename) {
  FILE *fp = fopen(filename, "r");
  SYSEXPECT(fp != NULL);
  int seek_ret;
  seek_ret = fseek(fp, 0, SEEK_END);
  SYSEXPECT(seek_ret == 0);
  int sz = ftell(fp);
  SYSEXPECT(seek_ret != -1L);
  seek_ret = fseek(fp, 0, SEEK_SET);
  SYSEXPECT(seek_ret == 0);
  char *buf = (char *)malloc(sz + 1);
  SYSEXPECT(buf);
  int read_ret = fread(buf, 1, sz, fp);
  if(read_ret != sz) {
    printf("fread() returns %d (expecting %d)\n", read_ret, sz);
    error_exit("File read error \"%s\"\n", filename);
  }
  // Make it a string
  buf[sz] = '\0'; 
  // Init file name
  parse_t *parse = _parse_init(buf);
  parse->filename = (char *)malloc(strlen(filename) + 1);
  SYSEXPECT(parse->filename != NULL);
  strcpy(parse->filename, filename);
  return parse;
}

void parse_free(parse_t *parse) {
  free(parse->s);
  free(parse->filename);
  free(parse);
  return;
}

// Always call this for reading the string, since we update line and col here
// Returns '\0' if the string is exhausted and do not change the pointer
char parse_getchar(parse_t *parse) {
  char ch = *parse->curr;
  if(ch != '\0') {
    parse->curr++;
    parse->col++;
    // Update line and column if we have seen a new line character
    if(ch == '\n') {
      parse->line++;
      parse->col = 0; // We always set col to zero before reading the first col
    }
  }
  return ch;
}

// This function skips spaces from the current pos, and stops at the next non-space char
// Could stop at '\0'
// Also jumps over line comments
void parse_skip_space(parse_t *parse) {
  while(1) {
    while(isspace(parse_peek(parse))) {
      parse_getchar(parse); // Skip current char
    }
    // Stray '#' always means line comment
    if(parse_peek(parse) == '#') {
      // Skip the current line, could also reach end of stream
      while(parse_peek(parse) != '\n' && parse_peek(parse) != '\0') {
        parse_getchar(parse);
      }
    } else {
      break;
    }
  }
  return;
}

// Returns the next non-space char in the stream, or '\0' if reaches the end
char parse_getchar_nospace(parse_t *parse) {
  parse_skip_space(parse);
  return parse_getchar(parse);
}

// Only peeks, not removing the char from the stream
char parse_peek_nospace(parse_t *parse) {
  parse_skip_space(parse);
  return parse_peek(parse);
}

// end points to the next char that should not be included
char *parse_copy(parse_t *parse, char *begin, char *end) {
  (void)parse;
  int len = (int)(end - begin);
  assert(len > 0);
  char *buf = (char *)malloc(len + 1);
  SYSEXPECT(buf != NULL);
  memcpy(buf, begin, len);
  buf[len] = '\0';
  return buf;
}

char *parse_get_ident(parse_t *parse) {
  parse_skip_space(parse);
  char *begin = parse->curr;
  if(*begin == '\0') return NULL; // If there is nothing to parse, return NULL
  // Check first char - special case since we do not allow number
  else if(!isalpha(*begin) && *begin != '_') return NULL;
  parse_getchar(parse);
  // This loops stops at the first char not constituting an ident
  while(1) {
    char c = parse_peek(parse);
    if(c == '\0') break;
    if(!isalnum(c) && c != '_') break;
    parse_getchar(parse);
  }
  // parse->curr always points to the next char that should not be included
  return parse_copy(parse, begin, parse->curr);
}

// Escape \" is not considered as a quotation mark
// This function reports error if any of the quotation marks are missing
char *parse_get_str(parse_t *parse) {
  parse_skip_space(parse);
  parse_expect_char(parse, '\"'); // Discards it if found; Report error if not
  // Guaranteed not the first char in the stream (we can look back by one)
  char *begin = parse->curr;
  char *end = NULL;
  while(1) {
    char ch = parse_peek(parse);
    if(ch == '\0') {
      parse_report_pos(parse);
      error_exit("Unexpected end of stream when reading a string\n");
    } else if(ch == '\\') {
      parse_getchar(parse);
      // Only process escaped here; Other cases are left to the next iter loop
      if(parse_peek(parse) == '\"') {
        parse_getchar(parse);
      }
    } else if(ch == '\"') {
      end = parse->curr;
      parse_getchar(parse);
      break;
    } else {
      parse_getchar(parse);
    }
  }
  return parse_copy(parse, begin, end);
}

uint32_t parse_get_color(parse_t *parse) {
  char *s = parse_get_str(parse);
  // This function prints error message, but we handle error by checking return value
  uint32_t ret = color_decode(s);
  free(s);
  if(ret == -1u) {
    parse_report_pos(parse);
    error_exit("Error decoding color\n");
  }
  return ret;
}

// Returns an allocated buffer containing the substring from the current position to the target ch, or '\0'
// ch itself is discarded from both the buffer and the input stream
// Caller should free the buffer
// Returns NULL if there is nothing to parse
// NOTE: This function does not work well with line comments
char *parse_until(parse_t *parse, char ch) {
  parse_skip_space(parse);
  char *begin = parse->curr;
  if(*begin == '\0') return NULL; // If there is nothing to parse, return NULL
  while(1) {
    char c = parse_getchar(parse);
    if(c == ch || c == '\0') break;
  }
  // If we matched the target char, the actual substring should be one less
  char *end = (*parse->curr == '\0') ? (parse->curr) : (parse->curr - 1);
  char *ret = parse_copy(parse, begin, end);
  // Right trim
  int len = end - begin;
  assert(len > 0);
  char *ret_end = ret + len - 1; // Last char
  while(isspace(*ret_end)) {
    *ret_end = '\0';
    ret_end--;
  }
  return ret;
}

double parse_get_double(parse_t *parse) {
  parse_skip_space(parse);
  char *begin = parse->curr;
  char *end = NULL;
  double ret = strtod(begin, &end);
  if(end == begin) {
    parse_report_pos(parse);
    error_exit("No valid conversion could be made to form a double\n");
  } else if(errno == ERANGE) {
    parse_report_pos(parse);
    error_exit("Double value out of range\n");
  }
  // Skip these characters
  while(parse->curr != end) {
    assert(parse->curr < end);
    parse_getchar(parse);
  }
  return ret;
}

double parse_get_double_range(parse_t *parse, double lower, double upper) {
  assert(lower <= upper);
  double ret = parse_get_double(parse);
  if(ret < lower || ret > upper) {
    parse_report_pos(parse);
    error_exit("Double must be within range [%g, %g] (sees %g)\n",
      lower, upper, ret);
  }
  return ret;
}

int64_t parse_get_int64(parse_t *parse) {
  parse_skip_space(parse);
  char *begin = parse->curr;
  char *end = NULL;
  int64_t ret = strtol(begin, &end, 10);
  if(end == begin) {
    parse_report_pos(parse);
    error_exit("No valid conversion could be made to form a int64_t\n");
  } else if(errno == ERANGE) {
    parse_report_pos(parse);
    error_exit("Integer value out of range\n");
  }
  // Skip these characters
  while(parse->curr != end) {
    assert(parse->curr < end);
    parse_getchar(parse);
  }
  return ret;
}

// Both lower and upper are inclusive, i.e. [lower, upper]
int64_t parse_get_int64_range(parse_t *parse, int64_t lower, int64_t upper) {
  assert(lower <= upper);
  int64_t ret = parse_get_int64(parse);
  if(ret < lower || ret > upper) {
    parse_report_pos(parse);
    error_exit("Integer must be within range [%ld, %ld] (sees %ld)\n",
      lower, upper, ret);
  }
  return ret;
}

// Reads file name and opens the file and returns the file pointer
// Files are indicated by "@"
char *parse_get_filename(parse_t *parse) {
  parse_expect_char(parse, '@');
  char *filename = parse_get_str(parse);
  return filename;
}

// buf should be at least 5 chars in size (\xHH\0)
static void parse_print_char(parse_t *parse, char ch, char *buf) {
  (void)parse;
  if(isprint(ch)) sprintf(buf, "'%c'", ch);
  else sprintf(buf, "\\x%02X", ch);
  return;
}

// Fetches the next non-space char in the stream; if this matches the given char then
// discard it. Otherwise report error
void parse_expect_char(parse_t *parse, char ch) {
  // We can use getchar here since it will be discarded anyway
  char c = parse_getchar_nospace(parse);
  if(c == '\0') {
    parse_report_pos(parse);
    char buf[6]; parse_print_char(parse, ch, buf);
    if(ch == ';') printf("Did you forget the trailing ';'?\n");
    error_exit("Expecting %s, while seeing end of stream\n", buf);
  } else if(c != ch) {
    parse_report_pos(parse);
    char buf[6]; parse_print_char(parse, ch, buf);
    if(ch == ';') printf("Did you forget the trailing ';'?\n");
    error_exit("Expecting %s, while seeing '%c'\n", buf, c);
  }
  return;
}

// Same as above, except that we do not read from stream and report error if mismatch
void parse_expect_char_opt(parse_t *parse, char ch) {
  char c = parse_peek_nospace(parse);
  if(c == ch) parse_getchar(parse); // Advance read pointer
  return;
}

// Returns argument type (STR, NUM, IDENT) and stop the parser at the next arg on the same line, if any.
// Otherwise, it returns 0 and stops at the terminating ';'
// If we reach the next line without seeing either a ';' or argument, report error
int parse_next_arg(parse_t *parse) {
  int line = parse->line;
  char ch = parse_peek_nospace(parse);
  if(line != parse->line || *parse->curr == '\0') {
    parse_report_pos(parse);
    error_exit("Did you miss a semicolon after function call?\n");
  }
  if(ch == ';') return 0;
  else if(ch == '\"') return PARSE_ARG_STR;
  else if(ch == '_' || isalpha(ch)) return PARSE_ARG_IDENT;
  else if(ch == '.' || isdigit(ch)) return PARSE_ARG_NUM; // hex 0xfff dec .decimal oct 0777
  else if(ch == '@') return PARSE_ARG_FILE;
  char buf[16];
  parse_print_char(parse, ch, buf);
  error_exit("Unknown argument character: %s\n", buf);
  return -1;
}

// Sort a given table
void parse_sort_cb(parse_t *parse, parse_cb_entry_t *table, int count) {
  (void)parse;
  for(int i = 0;i < count;i++) {
    for(int curr = 0;curr < count - 1;curr++) {
      int cmp = strcmp(table[curr].name, table[curr + 1].name);
      if(cmp == 0) {
        error_exit("Two functions have the same name: %s\n", table[curr].name);
      } else if(cmp > 0) {
        parse_cb_entry_t t = table[curr + 1];
        table[curr + 1] = table[curr];
        table[curr] = t;
      }
    }
  }
  return;
}

// Finds a call back given a name using binary search
// If not found the returned cb_entry_t object has name field set to NULL
parse_cb_entry_t parse_find_cb_entry(parse_t *parse, parse_cb_entry_t *table, int count, const char *name) {
  (void)parse;
  parse_cb_entry_t cb_entry;
  memset(&cb_entry, 0x00, sizeof(parse_cb_entry_t));
  // [begin, end)
  int begin = 0, end = count;
  while(begin < end) {
    int mid = (begin + end) / 2;
    int cmp = strcmp(name, table[mid].name);
    if(cmp == 0) {
      cb_entry = table[mid];
      break;
    } else if(cmp < 0) {
      end = mid;
    } else {
      begin = mid + 1;
    }
  }
  return cb_entry;
}

// Top-level parsing function
void parse_top(parse_t *parse, plot_t *plot) {
  char ch;
  while(1) {
    ch = parse_getchar_nospace(parse);
    switch(ch) {
      case '+': { // Entities
        parse_top_entity(parse, plot);
      } break; 
      case '.': { // Properties
        parse_top_property(parse, plot);
      } break;
      case '!': {
        parse_top_func(parse, plot);
      } break;
      case '\0': {
        goto func_ret; // This is actually the best way
      } break;
      default: {
        parse_report_pos(parse);
        char buf[8];
        parse_print_char(parse, ch, buf);
        error_exit("Unknown top-level directive: %s\n", buf);
      } break;
    }
  }
func_ret:
  return;
}

parse_cb_entry_t parse_cb_top_entities[] = {
  PARSE_GEN_CB("bar_type", parse_cb_bar_type),
  PARSE_GEN_CB("bar_group", parse_cb_bar_group),
};
const int parse_cb_top_entities_count = sizeof(parse_cb_top_entities) / sizeof(parse_cb_entry_t);

void parse_top_entity(parse_t *parse, plot_t *plot) {
  char *name = parse_get_ident(parse);
  if(name == NULL) {
    parse_report_pos(parse);
    error_exit("Expecting a entity name after top-level '+'\n");
  }
  // Table lookup
  parse_cb_entry_t cb_entry = parse_find_cb_entry(parse, parse_cb_top_entities, parse_cb_top_entities_count, name);
  if(cb_entry.name == NULL) {
    parse_report_pos(parse);
    error_exit("Unknown top-level entity: \"%s\"\n", name);
  } else {
    cb_entry.cb(parse, plot);
  }
  free(name);
  parse_expect_char(parse, ';');
  return;
}

// Bar type can be sprcified using scheme or in an ad-hoc manner
// Syntax:
// +bar_type "label" ["color"] ["hatch"]
// If one of or both color and/or hatch are not specified, we use the one from scheme, and increment offset pointer
// If color overflows, we report error
void parse_cb_bar_type(parse_t *parse, plot_t *plot) {
  (void)parse; (void)plot;
  printf("entity bar type\n");
  return;
}

void parse_cb_bar_group(parse_t *parse, plot_t *plot) {
  (void)parse; (void)plot;
  printf("entity bar group\n");
  return;
}

// This is the offset table, each having a pointer to parse_properties
parse_cb_entry_t parse_cb_top_props[] = {
  // plot_t fields
  PARSE_GEN_PROP("xtitle", PARSE_XTITLE),
  PARSE_GEN_PROP("ytitle", PARSE_YTITLE),
  PARSE_GEN_PROP("fig_filename", PARSE_FIG_FILENAME),
  PARSE_GEN_PROP("legend_filename", PARSE_LEGEND_FILENAME),
  // Size
  PARSE_GEN_PROP("width", PARSE_WIDTH),
  PARSE_GEN_PROP("height", PARSE_HEIGHT),
  // Legend
  PARSE_GEN_PROP("legend_rows", PARSE_LEGEND_ROWS),
  PARSE_GEN_PROP("legend_font_size", PARSE_LEGEND_FONT_SIZE),
  PARSE_GEN_PROP("legend_pos", PARSE_LEGEND_POS),
  // Ticks
  PARSE_GEN_PROP("xtick_font_size", PARSE_XTICK_FONT_SIZE),
  PARSE_GEN_PROP("xtick_rotation", PARSE_XTICK_ROTATION),
  PARSE_GEN_PROP("ytick_font_size", PARSE_YTICK_FONT_SIZE),
  PARSE_GEN_PROP("ytick_rotation", PARSE_YTICK_ROTATION),
  // Title
  PARSE_GEN_PROP("xtitle_font_size", PARSE_XTITLE_FONT_SIZE),
  PARSE_GEN_PROP("ytitle_font_size", PARSE_YTITLE_FONT_SIZE),
  // Bar text
  PARSE_GEN_PROP("bar_text_font_size", PARSE_BAR_TEXT_FONT_SIZE),
  PARSE_GEN_PROP("bar_text_rotation", PARSE_BAR_TEXT_ROTATION),
  PARSE_GEN_PROP("bar_text_decimals", PARSE_BAR_TEXT_DECIMALS),
  PARSE_GEN_PROP("bar_text_rtrim", PARSE_BAR_TEXT_RTRIM),
  // Limits
  PARSE_GEN_PROP("xlim_left", PARSE_XLIM_LEFT),
  PARSE_GEN_PROP("xlim_right", PARSE_XLIM_RIGHT),
  PARSE_GEN_PROP("ylim_top", PARSE_YLIM_TOP),
  PARSE_GEN_PROP("ylim_bottom", PARSE_YLIM_BOTTOM),
  // py_t object
  PARSE_GEN_PROP("dry_run", PARSE_DRY_RUN),
};
const int parse_cb_top_props_count = sizeof(parse_cb_top_props) / sizeof(parse_cb_entry_t);

// The "." has been removed from the stream
// We do not use jump table for this function, since most of the handlers are small and straightforward
void parse_top_property(parse_t *parse, plot_t *plot) {
  char *name = parse_get_ident(parse);
  if(name == NULL) {
    parse_report_pos(parse);
    error_exit("Expecting a property name after top-level '.'\n");
  }
  parse_expect_char(parse, '=');
  // Table lookup
  parse_cb_entry_t cb_entry = parse_find_cb_entry(parse, parse_cb_top_props, parse_cb_top_props_count, name);
  if(cb_entry.name == NULL) {
    parse_report_pos(parse);
    error_exit("Unknown top-level property: \"%s\"\n", name);
  } 
  // This should be compiled into a jump table
  switch(cb_entry.prop) {
    case PARSE_XTITLE: {
      plot->xtitle = parse_get_str(parse);
    } break;
    case PARSE_YTITLE: {
      plot->ytitle = parse_get_str(parse);
    } break;
    case PARSE_FIG_FILENAME: {
      if(plot->fig_filename != NULL) {
        printf("[parse] WARNING: The property \"fig_filename\" already exists, value \"%s\"\n", 
          plot->fig_filename);
        free(plot->fig_filename);
      }
      plot->fig_filename = parse_get_str(parse);
    } break;
    case PARSE_LEGEND_FILENAME: {
      if(plot->legend_filename != NULL) {
        printf("[parse] WARNING: The property \"legend_filename\" already exists, value \"%s\"\n", 
          plot->legend_filename);
        free(plot->legend_filename);
      }
      plot->legend_filename = parse_get_str(parse);
    } break;
    case PARSE_WIDTH: {
      plot->param.width = parse_get_double_range(parse, 0.0, PARSE_DOUBLE_MAX);
    } break;
    case PARSE_HEIGHT: {
      plot->param.height = parse_get_double_range(parse, 0.0, PARSE_DOUBLE_MAX);
    } break;
    case PARSE_LEGEND_ROWS: {
      plot->param.legend_rows = (int)parse_get_int64_range(parse, 1, PARSE_INT64_MAX);
    } break;
    case PARSE_LEGEND_FONT_SIZE: {
      plot->param.legend_font_size = (int)parse_get_int64_range(parse, 1, PARSE_INT64_MAX);
    } break;
    case PARSE_LEGEND_POS: {
      char *pos = parse_get_str(parse);
      plot_set_legend_pos(plot, pos); // Copies the string (report error if too long)
      free(pos); // We already copied the pos string into param
    } break;
    case PARSE_XTICK_FONT_SIZE: {
      plot->param.xtick_font_size = (int)parse_get_int64_range(parse, 1, PARSE_INT64_MAX);
    } break;
    case PARSE_XTICK_ROTATION: {
      plot->param.xtick_rotation = (int)parse_get_int64_range(parse, 0, 359);
    } break;
    case PARSE_YTICK_FONT_SIZE: {
      plot->param.ytick_font_size = (int)parse_get_int64_range(parse, 1, PARSE_INT64_MAX);
    } break;
    case PARSE_YTICK_ROTATION: {
      plot->param.ytick_rotation = (int)parse_get_int64_range(parse, 0, 359L);
    } break;
    case PARSE_XTITLE_FONT_SIZE: {
      plot->param.xtitle_font_size = (int)parse_get_int64_range(parse, 1, PARSE_INT64_MAX);
    } break;
    case PARSE_YTITLE_FONT_SIZE: {
      plot->param.ytitle_font_size = (int)parse_get_int64_range(parse, 1, PARSE_INT64_MAX);
    } break;
    case PARSE_BAR_TEXT_FONT_SIZE: {
      plot->param.bar_text_font_size = (int)parse_get_int64_range(parse, 1, PARSE_INT64_MAX);
    } break;
    case PARSE_BAR_TEXT_ROTATION: {
      plot->param.bar_text_rotation = (int)parse_get_int64_range(parse, 0, 359L);
    } break;
    case PARSE_BAR_TEXT_DECIMALS: {
      plot->param.bar_text_decimals = (int)parse_get_int64(parse);
    } break;
    case PARSE_BAR_TEXT_RTRIM: {
      plot->param.bar_text_rtrim = (int)parse_get_int64_range(parse, 0, 1);
    } break;
    case PARSE_XLIM_LEFT: {
      plot->param.xlim_left = parse_get_double_range(parse, PARSE_DOUBLE_MIN, PARSE_DOUBLE_MAX);
    } break;
    case PARSE_XLIM_RIGHT: {
      plot->param.xlim_right = parse_get_double_range(parse, PARSE_DOUBLE_MIN, PARSE_DOUBLE_MAX);
    } break;
    case PARSE_YLIM_TOP: {
      plot->param.ylim_top = parse_get_double_range(parse, PARSE_DOUBLE_MIN, PARSE_DOUBLE_MAX);
    } break;
    case PARSE_YLIM_BOTTOM: {
      plot->param.ylim_bottom = parse_get_double_range(parse, PARSE_DOUBLE_MIN, PARSE_DOUBLE_MAX);
    } break;
    case PARSE_DRY_RUN: {
      int prev = plot->param.dry_run;
      plot->param.dry_run = (int)parse_get_int64_range(parse, 0, 1);
      if(prev == 0 && plot->param.dry_run == 1) {
        printf("[parse] Dry run mode enabled; Scripts will not be actually executed\n");
      } else if(prev == 1 && plot->param.dry_run == 0) {
        printf("[parse] Dry run mode disabled; Scripts will be executed\n");
      }
    } break;
    default: {
      parse_report_pos(parse);
      error_exit("Internal error: unknown top-level property handler: \"%s\" (code %d)\n", name, cb_entry.prop);
    }
  }
  // This is common for all properties
  parse_expect_char(parse, ';');
  free(name);
  return;
}

parse_cb_entry_t parse_cb_top_funcs[] = {
  PARSE_GEN_CB("print", parse_cb_print),
  PARSE_GEN_CB("reset", parse_cb_reset),
  PARSE_GEN_CB("save_fig", parse_cb_save_fig),
  PARSE_GEN_CB("save_legend", parse_cb_save_legend),
  PARSE_GEN_CB("create_fig", parse_cb_create_fig),
  PARSE_GEN_CB("set_hatch_scheme", parse_cb_set_hatch_scheme),
  PARSE_GEN_CB("set_color_scheme", parse_cb_set_color_scheme),
  PARSE_GEN_CB("test_hatch", parse_cb_test_hatch),
  PARSE_GEN_CB("test_color", parse_cb_test_color),
};
const int parse_cb_top_funcs_count = sizeof(parse_cb_top_funcs) / sizeof(parse_cb_entry_t);

void parse_top_func(parse_t *parse, plot_t *plot) {
  char *name = parse_get_ident(parse);
  if(name == NULL) {
    parse_report_pos(parse);
    error_exit("Expecting a function name after top-level '!'\n");
  } 
  parse_cb_entry_t cb_entry = parse_find_cb_entry(parse, parse_cb_top_funcs, parse_cb_top_funcs_count, name);
  if(cb_entry.name == NULL) {
    parse_report_pos(parse);
    error_exit("Unknown top-level function: \"%s\"\n", name);
  } else {
    cb_entry.cb(parse, plot);
    parse_expect_char(parse, ';');
  }
  free(name);
  return;
}

void parse_cb_print(parse_t *parse, plot_t *plot) {
  if(parse_next_arg(parse) == PARSE_ARG_NONE) {
    parse_report_pos(parse);
    error_exit("Function \"print\" expects at least 1 argument\n");
  }
  char *name = parse_get_ident(parse);
  int verbose = 0;
  if(parse_next_arg(parse) != PARSE_ARG_NONE) {
    char *verbose_ident = parse_get_ident(parse);
    if(streq(verbose_ident, "verbose") == 1) {
      verbose = 1;
    } else {
      parse_report_pos(parse);
      error_exit("Unknown option for \"print\": \"%s\"\n", verbose_ident);
    }
    free(verbose_ident);
  }
  if(streq(name, "plot") == 1) {
    plot_print(plot, verbose);
  } else if(streq(name, "param") == 1) {
    plot_param_print(&plot->param, verbose);
  } else if(streq(name, "version") == 1) {
    printf("[version] matplotlib C language wrapper and script interpreter, version %s.%s\n", 
      MAJOR_VERSION, MINOR_VERSION);
    printf("[version] Author: Ziqi Wang\n");
    printf("[version] Github: https://github.com/wangziqi2016/matplotlib-c\n");
    if(verbose == 1) {
#ifdef __GNUC__
      printf("[version] GCC version: %d.%d.%d\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#endif
#ifdef __DATE__
      printf("[version] Date: %s\n", __DATE__);
#endif
#ifdef __TIME__
      printf("[version] Time: %s\n", __TIME__);
#endif
#ifdef __FILE__
      printf("[version] File: %s\n", __FILE__);
#endif
    }
  }
  free(name);
  return;
}

void parse_cb_reset(parse_t *parse, plot_t *plot) {
  if(parse_next_arg(parse) == PARSE_ARG_NONE) {
    parse_report_pos(parse);
    error_exit("Function \"reset\" expects 1 argument\n");
  }
  // This can be "buf" "param" or "plot"
  char *name = parse_get_ident(parse);
  plot_param_t *param = &plot->param;
  int reset_buf = 0, reset_param = 0;
  if(streq(name, "buf") == 1) {
    reset_buf = 1;
  } else if(streq(name, "param") == 1) {
    reset_param = 1;
  } else if(streq(name, "plot") == 1) {
    reset_buf = reset_param = 1; // Reset both
  }
  if(reset_buf == 1) {
    buf_reset(plot->buf);
    buf_append(plot->buf, plot_preamble);
  }
  if(reset_param == 1) {
    if(param->color_scheme != NULL) {
      printf("[parse] Color scheme \"%s\" will be removed from plot during reset\n", param->color_scheme->name);
    }
    if(param->hatch_scheme != NULL) {
      printf("[parse] Hatch scheme \"%s\" will be removed from plot during reset\n", param->hatch_scheme->name);
    }
    plot_param_copy(&plot->param, &default_param);
  }
  free(name);
  return;
}

void parse_cb_save_fig(parse_t *parse, plot_t *plot) {
  printf("Save fig called!\n");
  return;
  char *filename = NULL; // Given in arg list
  if(parse_next_arg(parse)) {
    filename = parse_get_str(parse);
    if(plot->fig_filename != NULL) {
      printf("[parse] Overriding existing save fig filename: \"%s\"\n", plot->fig_filename);
    }
    plot_save_fig(plot, filename);
    free(filename);
  } else {
    if(plot->fig_filename == NULL) {
      error_exit("Need a file name to save the figure (either as property or argument)\n");
    }
    plot_save_fig(plot, plot->fig_filename);
  }
  if(parse_next_arg(parse)) {
    error_exit("Function \"save_fig\" takes 1 optional argument\n");
  }
  return;
}

void parse_cb_save_legend(parse_t *parse, plot_t *plot) {
  printf("Save legend called!\n");
  return;
  char *filename = NULL;
  if(parse_next_arg(parse)) {
    filename = parse_get_str(parse);
    if(plot->legend_filename != NULL) {
      printf("[parse] Overriding existing save legend filename: \"%s\"\n", plot->legend_filename);
    }
    plot_save_legend(plot, filename);
    free(filename);
  } else {
    if(plot->legend_filename == NULL) {
      error_exit("Need a file name to save the legend (either as property or argument)\n");
    }
    plot_save_legend(plot, plot->legend_filename);
  }
  if(parse_next_arg(parse)) {
    error_exit("Function \"save_legend\" takes 1 optional argument\n");
  }
  return;
}

// This function has two optional arguments: width height
// If only one is given then it defaults to width
void parse_cb_create_fig(parse_t *parse, plot_t *plot) {
  double width = plot->param.width;
  double height = plot->param.height;
  if(parse_next_arg(parse)) {
    double new_width = parse_get_double_range(parse, 0.0, PARSE_DOUBLE_MAX);
    printf("[parse] Overriding param width %g with %g\n", width, new_width);
    width = new_width;
  } 
  if(parse_next_arg(parse)) {
    double new_height = parse_get_double_range(parse, 0.0, PARSE_DOUBLE_MAX);
    printf("[parse] Overriding param height %g with %g\n", height, new_height);
    height = new_height;
  }
  if(parse_next_arg(parse)) {
    error_exit("Function \"create_fig\" takes 1 or 2 optional arguments\n");
  }
  plot_create_fig(plot, width, height);
  return;
}

void parse_cb_set_hatch_scheme(parse_t *parse, plot_t *plot) {
  int next_type = parse_next_arg(parse);
  if(next_type == PARSE_ARG_NONE) {
    error_exit("Function \"set_hatch_scheme\" takes at least 1 argument\n");
  }
  plot_param_t *param = &plot->param;
  // Free current one if there is one
  if(param->hatch_scheme != NULL) {
    printf("[parse] Overriding existing hatch scheme \"%s\"\n", param->hatch_scheme->name);
    hatch_scheme_free(param->hatch_scheme);
  }
  if(next_type == PARSE_ARG_STR) {
    char *scheme_name = parse_get_str(parse);
    hatch_scheme_t *scheme = hatch_find_scheme(scheme_name);
    if(scheme == NULL) {
      error_exit("Hatch scheme name \"%s\" does not exist\n", scheme_name);
    }
    free(scheme_name);
    param->hatch_scheme = hatch_scheme_init(scheme->name, scheme->base, scheme->item_count);
  } else if(next_type == PARSE_ARG_FILE) {
    char *filename = parse_get_filename(parse);
    param->hatch_scheme = hatch_scheme_init_file(filename);
    if(param->hatch_scheme == NULL) {
      parse_report_pos(parse);
      error_exit("Failed to read file \"%s\" for hatch scheme\n", filename);
    }
    free(filename);
  }
  // Read hatch offset
  if(parse_next_arg(parse)) {
    plot->param.hatch_offset = parse_get_int64_range(parse, 0, plot->param.hatch_scheme->item_count - 1);
  }
  if(parse_next_arg(parse)) {
    error_exit("Function \"set_hatch_scheme\" takes 1 or 2 arguments\n");
  }
  return;
}

void parse_cb_set_color_scheme(parse_t *parse, plot_t *plot) {
  int next_type = parse_next_arg(parse);
  if(next_type == PARSE_ARG_NONE) {
    error_exit("Function \"set_color_scheme\" takes at least 1 argument\n");
  }
  plot_param_t *param = &plot->param;
  // Free current one if there is one
  if(param->color_scheme != NULL) {
    printf("[parse] Overriding existing color scheme \"%s\"\n", param->color_scheme->name);
    color_scheme_free(param->color_scheme);
  }
  if(next_type == PARSE_ARG_STR) {
    char *scheme_name = parse_get_str(parse);
    color_scheme_t *scheme = color_find_scheme(scheme_name);
    if(scheme == NULL) {
      error_exit("Color scheme name \"%s\" does not exist\n", scheme_name);
    }
    free(scheme_name);
    // Copy initialize the hardcoded color scheme
    param->color_scheme = color_scheme_init(scheme->name, scheme->base, scheme->item_count);
  } else if(next_type == PARSE_ARG_FILE) {
    char *filename = parse_get_filename(parse);
    param->color_scheme = color_scheme_init_file(filename);
    if(param->color_scheme == NULL) {
      parse_report_pos(parse);
      error_exit("Failed to read file \"%s\" for color scheme\n", filename);
    }
    free(filename);
  }
  // Read hatch offset
  if(parse_next_arg(parse)) {
    param->color_offset = parse_get_int64_range(parse, 0, param->color_scheme->item_count - 1);
  }
  if(parse_next_arg(parse)) {
    error_exit("Function \"set_color_scheme\" takes 1 or 2 arguments\n");
  }
  return;
}

void parse_cb_test_hatch(parse_t *parse, plot_t *plot) {
  if(parse_next_arg(parse) == 0) {
    error_exit("Function \"test_hatch\" takes 1 argument\n");
  }
  char *filename = parse_get_str(parse);
  if(parse_next_arg(parse)) {
    error_exit("Function \"test_hatch\" only takes 1 argument\n");
  }
  printf("[parse] Saving hatch test file to \"%s\"\n", filename);
  plot_save_hatch_test(plot, filename);
  free(filename);
  return;
}

void parse_cb_test_color(parse_t *parse, plot_t *plot) {
  if(parse_next_arg(parse) == 0) {
    error_exit("Function \"test_color\" takes 1 argument\n");
  }
  char *filename = parse_get_str(parse);
  if(parse_next_arg(parse)) {
    error_exit("Function \"test_color\" only takes 1 argument\n");
  }
  printf("[parse] Saving color test file to \"%s\"\n", filename);
  plot_save_color_test(plot, filename);
  free(filename);
  return;
}

// Prints the value of a property into the given buf
// This function uses the same format string as printf
void parse_print_prop(parse_t *parse, buf_t *buf, const char *name, const char *fmt) {

}

// Prints a string, which can possibly be a format string containing format specifiers
// The parser will keep reading arguments for each specifier except %%
void parse_print_str(parse_t *parse, buf_t *buf, const char *str) {
  char *p = str;
  while(*p != '\0') {

  }
}

// Reports current line and col followed by the current line; Used in error reporting
void parse_report_pos(parse_t *parse) {
  printf("File %s on line %d column %d: \n", parse->filename, parse->line, parse->col);
  printf("  \"");
  char *p = parse->curr;
  // Rewind to beginning of line or beginning of stream, whichever comes first
  while(p != parse->s && p[-1] != '\n') p--;
  do {
    if(isprint(*p)) putchar(*p);
    else if(*p == '\n') printf("\\n");
    else if(*p == '\r') printf("\\r");
    else if(*p == '\0') printf("\\0");
    else printf("\\x%02X", (int)*p);
    p++;
  } while(*p != '\n' && *p != '\0');
  printf("\"\n");
  return;
}

void parse_print(parse_t *parse) {
  printf("[parse_t] size %d line %d col %d offset %d s 0x%p\n", 
    parse->size, parse->line, parse->col, (int)(parse->curr - parse->s), parse->s);
  return;
}
