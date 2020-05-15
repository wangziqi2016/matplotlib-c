
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

// Returns the hatch char given a string (basically containing the hatch char)
// Similar to color_decode, this function returns 0xFF if error, prints error message, and caller
// should actually handle the error
char hatch_decode(const char *s) {
  if(strlen(s) != 1) {
    printf("Hatch should be only one character in length (see \"%s\")\n", s);
    return 0xFF;
  } else if(hatch_is_valid(s[0]) == 0) {
    printf("Invalid hatch character: \'%c\'\n", s[0]);
    return 0xFF;
  }
  return s[0];
}

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
  // This is to avoid having bar plotting params overwritten
  bar->inited = 0;
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
    bar->type ? bar->type->label : "N/A");
  printf("        inited %d stacked %d\n", bar->inited, bar->stacked);
  return;
}

//* bargrp_t

bargrp_t *bargrp_init(const char *name) {
  bargrp_t *grp = (bargrp_t *)malloc(sizeof(bargrp_t));
  SYSEXPECT(grp != NULL);
  memset(grp, 0x00, sizeof(bargrp_t));
  grp->name = (char *)malloc(strlen(name) + 1);
  SYSEXPECT(grp->name != NULL);
  strcpy(grp->name, name);
  grp->bars = vec_init();
  return grp;
}

void bargrp_free(bargrp_t *grp) {
  free(grp->name);
  // Free bars and the vector itself
  for(int i = 0;i < vec_count(grp->bars);i++) {
    bar_free(vec_at(grp->bars, i));
  }
  vec_free(grp->bars);
  free(grp);
  return;
}

void bargrp_print(bargrp_t *grp, int verbose) {
  printf("[bargrp] name \"%s\" size %d\n", grp->name, vec_count(grp->bars));
  if(verbose == 1) {
    for(int i = 0;i < vec_count(grp->bars);i++) {
      bar_print(vec_at(grp->bars, i));
    }
  }
  return;
}

//* plot_t

plot_param_t default_param = {
  12.0, 6.0, // Width and Height
  1,         // legend_enabled
  1,         // legend_rows
  28,        // legend_font_size
  "best",    // Legend pos; Alternatives are: {lower, center, upper} x {left, center, right} or "center"
  1, INFINITY, PLOT_DIRECTION_INSIDE, 24, 0, 1,  // x tick enabled, length, dir, font size, rotation, label enabled
  1, INFINITY, PLOT_DIRECTION_INSIDE, 24, 0, 1,  // y tick enabled, length, dir, font size, rotation, label enabled
  0,         // x grid enabled
  0,         // y grid enabled
  28, 28,    // x/y title font size
  26, 90,    // bar text size, rotation
  2, 1,      // bar text decimals, rtrim
  NULL, 0,   // Hatch scheme/offset
  NULL, 0,   // Color scheme/offset
  INFINITY, INFINITY, // xlimits
  INFINITY, INFINITY, // ylimits
  1.0,       // bargrp space
  0,         // Dry run
  1,         // Info
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
  printf("[param legend] enabled %d font size %d rows %d pos \"%s\"\n", 
    param->legend_enabled, param->legend_font_size, param->legend_rows, param->legend_pos);
  printf("[param title] x font %d y font %d\n", 
    param->xtitle_font_size, param->ytitle_font_size);
  printf("[param xtick] enabled %d len %f dir %d font %d rot %d label %d\n", 
    param->xtick_enabled, param->xtick_length, param->xtick_direction, param->xtick_font_size, param->xtick_rotation,
    param->xtick_label_enabled);
  printf("[param ytick] enabled %d len %f dir %d font %d rot %d label %d\n", 
    param->ytick_enabled, param->ytick_length, param->ytick_direction, param->ytick_font_size, param->ytick_rotation,
    param->ytick_label_enabled);
  printf("[param xgrid] enabled %d\n",
    param->xgrid_enabled);
  printf("[param ygrid] enabled %d\n",
    param->ygrid_enabled);
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
  printf("[param bargrp] space %f\n", param->bargrp_space);
  printf("[debug] dry_run %d info %d\n", param->dry_run, param->info);
  return;
}

//* plot_tick_t

plot_tick_t *plot_tick_init() {
  plot_tick_t *tick = (plot_tick_t *)malloc(sizeof(plot_tick_t));
  SYSEXPECT(tick != NULL);
  memset(tick, 0x00, sizeof(plot_tick_t));
  tick->poses = vec_init();
  tick->labels = vec_init();
  if(sizeof(double) != 8UL) error_exit("The size of a double must be 8 bytes\n");
  return tick;
}

void plot_tick_free(plot_tick_t *tick) {
  // Free label strings first
  for(int i = 0;i < vec_count(tick->labels);i++) {
    free((char *)vec_at(tick->labels, i));
  }
  vec_free(tick->labels);
  vec_free(tick->poses);
  free(tick);
  return;
}

void plot_tick_append(plot_tick_t *tick, double pos, const char *str) {
  uint64_t t = *(uint64_t *)&pos;
  vec_append(tick->poses, (void *)t);
  // Duplicate the string and push into the vector
  int size = strlen(str) + 1;
  char *s = (char *)malloc(size);
  SYSEXPECT(s != NULL);
  memcpy(s, str, size);
  vec_append(tick->labels, s);
  assert(vec_count(tick->poses) == vec_count(tick->labels));
  return;
}

double plot_tick_pos_at(plot_tick_t *tick, int index) {
  assert(index >= 0 && index < vec_count(tick->poses));
  // Must do this to let compiler perform binary conversion
  uint64_t t = (uint64_t)vec_at(tick->poses, index);
  return *(double *)&t;
}

char *plot_tick_label_at(plot_tick_t *tick, int index) {
  assert(index >= 0 && index < vec_count(tick->poses));
  return (char *)vec_at(tick->labels, index);
}

buf_t *plot_tick_pos_str(plot_tick_t *tick) {
  buf_t *buf = buf_init();
  buf_append(buf, "[");
  for(int i = 0;i < vec_count(tick->poses);i++) {
    buf_printf(buf, "%f, ", plot_tick_pos_at(tick, i));
  }
  buf_append(buf, "]");
  return buf;
}

buf_t *plot_tick_label_str(plot_tick_t *tick) {
  buf_t *buf = buf_init();
  buf_append(buf, "[");
  for(int i = 0;i < vec_count(tick->poses);i++) {
    buf_printf(buf, "'%s', ", plot_tick_label_at(tick, i));
  }
  buf_append(buf, "]");
  return buf;
}

void plot_tick_print(plot_tick_t *tick, int verbose) {
  int count = vec_count(tick->poses);
  printf("[plot_tick] count %d\n", count);
  if(verbose == 1) {
    for(int i = 0;i < count;i++) {
      printf("[plot_tick] Index %d pos %f label \"%s\"\n", 
        i, plot_tick_pos_at(tick, i), plot_tick_label_at(tick, i));
    }
  }
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
  plot->bargrps = vec_init();
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
  // Free bar groups and the vector
  for(int i = 0;i < vec_count(plot->bargrps);i++) {
    bargrp_free(vec_at(plot->bargrps, i));
  }
  vec_free(plot->bargrps);
  // Free current bar group if not NULL. This should not be normal, however
  if(plot->curr_bargrp != NULL) {
    printf("WARNING: curr_bargrp is not NULL. Some bargroups are not added for plotting\n");
    bargrp_free(plot->curr_bargrp);
  }
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
  free(plot);
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
    error_exit("Bar type label \"%s\" already exists\n", label);
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

// This function adds a bar group into bargrps; Names must be unique
void plot_add_bargrp(plot_t *plot, bargrp_t *grp) {
  if(plot_find_bargrp(plot, grp->name) != NULL) {
    error_exit("The bar group name \"%s\" already exists\n", grp->name);
  }
  vec_append(plot->bargrps, (void *)grp);
  return;
}

// Returns NULL if not found
bargrp_t *plot_find_bargrp(plot_t *plot, const char *name) {
  for(int i = 0;i < vec_count(plot->bargrps);i++) {
    bargrp_t *grp = (bargrp_t *)vec_at(plot->bargrps, i);
    if(streq(grp->name, name) == 1) {
      return grp;
    }
  }
  return NULL;
}

// Reset flags that will be set during draw() such that we can draw again
void plot_reset_flags(plot_t *plot) {
  // Reset types
  for(int i = 0;i < vec_count(plot->bar_types);i++) {
    bar_type_t *type = vec_at(plot->bar_types, i);
    type->used = 0;
  }
  // Reset bars
  for(int i = 0;i < vec_count(plot->bargrps);i++) {
    bargrp_t *grp = (bargrp_t *)vec_at(plot->bargrps, i);
    for(int j = 0;j < vec_count(grp->bars);j++) {
      bar_t *bar = vec_at(grp->bars, j);
      bar->inited = 0;
    }
  }
  return;
}

// Reset the buffer to restart a new plotting
void plot_reset_buf(plot_t *plot) {
  buf_reset(plot->buf);
  buf_append(plot->buf, plot_preamble);
  return;
}

// This mainly initializes the axis object with its dimension
void plot_draw_axis(plot_t *plot) {
  plot_param_t *param = &plot->param;
  buf_printf(plot->buf, "fig = plot.figure(figsize=(%f, %f))\n", param->width, param->height);
  // "111" means the output consists of only one plot
  buf_append(plot->buf, "ax = fig.add_subplot(111)\n\n");
  return;
}

// Generates plotting script for an individual bar, assuming parameters are already set
// This function could be called without adding bar and bar groups to the plot object
// Only one new line is appended at the end of the draw
void plot_draw_bar(plot_t *plot, bar_t *bar) {
  assert(bar->type != NULL);
  buf_t *buf = plot->buf;
  plot_param_t *param = &plot->param;
  if(bar->inited == 0) {
    error_exit("Bar is not initialized; Did you initialize properly? "
               "(call plot_draw_all_bars() to compute automatically)\n");
  }
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

// This function calls plot_draw_bar() to draw individual bars
// Also adds ticks into the tick array
void plot_draw_all_bargrps(plot_t *plot) {
  plot_param_t *param = &plot->param;
  if(plot->curr_bargrp != NULL && param->info == 1) {
    printf("[plot] WARNING: curr_bargrp is not NULL; Some bars may not be plotted\n");
  }
  int bar_count = 0;
  int space_count = 2 + (vec_count(plot->bargrps) - 1);
  for(int i = 0;i < vec_count(plot->bargrps);i++) {
    bargrp_t *grp = (bargrp_t *)vec_at(plot->bargrps, i);
    bar_count += bargrp_count(grp);
  }
  if(bar_count == 0) {
    if(param->info == 1) printf("[plot] There is no bar to plot\n");
    return;
  }
  // Since bargrp space may be less than bar width
  double effective_bar_count = (double)bar_count + (double)space_count * param->bargrp_space;
  double bar_width = param->width / effective_bar_count;
  double curr_pos = bar_width * param->bargrp_space;
  double curr_bottom = 0.0;
  for(int i = 0;i < vec_count(plot->bargrps);i++) {
    bargrp_t *grp = (bargrp_t *)vec_at(plot->bargrps, i);
    for(int j = 0;j < vec_count(grp->bars);j++) {
      bar_t *bar = vec_at(grp->bars, j);
      if(bar->inited == 1 && param->info == 1) {
        printf("WARNING: The bar object's width and/or pos has been initialized\n");
      }
      bar->inited = 1;
      bar->width = bar_width;
      bar->pos = curr_pos;
      bar->bottom = curr_bottom;
      int is_last = (j == (vec_count(grp->bars) - 1));
      int next_stacked = (is_last || (((bar_t *)vec_at(grp->bars, j + 1))->stacked == 0));
      if(is_last == 1) {
        // Either jump to the next immediate slot, or leave inter-group space
        curr_pos += (bar_width + bar_width * param->bargrp_space);
        curr_bottom = 0.0;
      } else if(next_stacked == 1) {
        curr_bottom += bar->height;
      } else {
        // Regular next bar
        curr_pos += bar_width;
        curr_bottom = 0.0;
      }
      // Finally call the function to draw bars
      plot_draw_bar(plot, bar);
    }
  }
  return;
}

// Generates tick plotting code to the buffer using tick parameters
void plot_draw_tick(plot_t *plot) {
  buf_t *buf = plot->buf;
  plot_param_t *param = &plot->param;
  // Set x tick and y tick
  if(param->xtick_enabled == 1) {
    buf_printf(buf, "if len(cmatplotlib_xticks) != 0:\n");
    buf_printf(buf, "  plot.xticks(cmatplotlib_xticks, cmatplotlib_xtick_labels");
    buf_printf(buf, ", fontsize=%d", param->xtick_font_size);
    if(param->xtick_rotation != 0) {
      //printf("rotation %d\n", param->xtick_rotation);
      buf_printf(buf, ", rotation=%d", param->xtick_rotation);
    }
    buf_printf(buf, ")\n");
    // Print length
    buf_printf(buf, "ax.tick_params(axis='x', which='both'");
    if(param->xtick_length != INFINITY) {
      buf_printf(buf, ", length=%f", param->xtick_length);
    }
    if(param->xtick_direction == PLOT_DIRECTION_OUTSIDE) {
      buf_printf(buf, ", direction='out'");
    } else if(param->xtick_direction == PLOT_DIRECTION_BOTH) {
      buf_printf(buf, ", direction='inout'");
    }
    if(param->xtick_label_enabled == 0) buf_printf(buf, ", labelbottom=False");
    else buf_printf(buf, ", labelbottom=True");
    buf_printf(buf, ")\n");
  } else {
    buf_printf(buf, "plot.xticks([])\n");
  }
  if(param->ytick_enabled == 1) {
    // Print Y label
    buf_printf(buf, "if len(cmatplotlib_yticks) != 0:\n");
    buf_printf(buf, "  plot.yticks(cmatplotlib_yticks, cmatplotlib_ytick_labels");
    buf_printf(buf, ", fontsize=%d", param->ytick_font_size);
    if(param->xtick_rotation != 0) {
      buf_printf(buf, ", rotation=%d", param->ytick_rotation);
    }
    buf_printf(buf, ")\n");
    // Print length
    buf_printf(buf, "ax.tick_params(axis='y', which='both'");
    if(param->ytick_length != INFINITY) {
      buf_printf(buf, ", length=%f", param->ytick_length);
    }
    if(param->ytick_direction == PLOT_DIRECTION_OUTSIDE) {
      buf_printf(buf, ", direction='out'");
    } else if(param->ytick_direction == PLOT_DIRECTION_BOTH) {
      buf_printf(buf, ", direction='inout'");
    }
    if(param->ytick_label_enabled == 0) buf_printf(buf, ", labelleft=False");
    else buf_printf(buf, ", labelleft=True");
    buf_printf(buf, ")\n");
  } else { 
    buf_printf(buf, "plot.yticks([])\n");
  }
  //printf("****** BUFFER CONTENT\n");
  //printf("%s\n", buf_c_str(buf));
  return;
}

// Generates grid code before the plot is saved
void plot_draw_grid(plot_t *plot) {
  buf_t *buf = plot->buf;
  plot_param_t *param = &plot->param;
  if(param->xgrid_enabled == 1) {
    if(param->xtick_enabled == 0) printf("[plot] X tick is disabled. X grid will not be shown\n");
    buf_printf(buf, "plot.grid(b=True, axis='x'");
    // Here goes X grid customization
    buf_printf(buf, ")\n");
  }
  if(param->ygrid_enabled == 1) {
    if(param->ytick_enabled == 0) printf("[plot] Y tick is disabled. Y grid will not be shown\n");
    buf_printf(buf, "plot.grid(b=True, axis='y'");
    // Here goes Y grid customization
    buf_printf(buf, ")\n");
  }
  return;
}

void plot_draw_limit(plot_t *plot) {
  buf_t *buf = plot->buf;
  plot_param_t *param = &plot->param;
  if(param->xlim_left != INFINITY) buf_printf(buf, "ax.set_xlim(left=%f)\n", param->xlim_left);
  if(param->xlim_right != INFINITY) buf_printf(buf, "ax.set_xlim(right=%f)\n", param->xlim_right);
  if(param->ylim_top != INFINITY) buf_printf(buf, "ax.set_ylim(top=%f)\n", param->ylim_top);
  if(param->ylim_bottom != INFINITY) buf_printf(buf, "ax.set_ylim(bottom=%f)\n", param->ylim_bottom);
  return;
}

// Uses legend font size, legend vertical, and legend position in the param object
void plot_draw_legend(plot_t *plot) {
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

// Generates all scripts except save figure or show figure
void plot_draw(plot_t *plot) {
  plot_param_t *param = &plot->param;
  // First reset all flags and buf such that we can restart
  // Params are not reset here
  plot_reset_flags(plot);
  plot_reset_buf(plot);
  // Note that these can be skipped by arranging bars manually
  plot_draw_axis(plot);
  plot_draw_all_bargrps(plot);
  plot_draw_tick(plot);
  plot_draw_grid(plot);
  plot_draw_limit(plot);
  if(param->legend_enabled == 1) plot_draw_legend(plot);
  return;
}

// This function generates the code to save figure, and runs python interpreter
void plot_save_fig(plot_t *plot, const char *filename) {
  buf_t *buf = plot->buf;
  plot_param_t *param = &plot->param;
  // Print draw command and execute script
  if(param->dry_run == PLOT_DRY_RUN_DISABLED) {
    // Pass the file name
    buf_printf(buf, "plot.savefig(\"%s\", bbox_inches='tight')\n\n", filename);
    py_run(plot->py, buf_c_str(plot->buf));
  } else if(param->dry_run == PLOT_DRY_RUN_ENABLED) {
    buf_printf(buf, "plot.savefig(\"%s\", bbox_inches='tight')\n\n", filename);
    if(plot->param.info == 1) printf("[plot] Dry run mode is on; not executing anything\n");
  } else if(param->dry_run == PLOT_DRY_RUN_SHOW) {
    if(plot->param.info == 1) printf("[plot] Dry run mode is \"show\", showing the plot on-screen\n");
    buf_printf(buf, "plot.show()\n\n", filename);
    py_run(plot->py, buf_c_str(buf));
  }
  return;
}

// Saves a standalone legend file
// This function can be called anywhere during the plotting procedure. Legends drawn will be 
// bar types stored in the plot object. Report error if there is not any bar type.
void plot_save_legend_mode(plot_t *plot, int mode, void *arg) {
  plot_t *legend = plot_init(); // Preamble is set after this
  plot_param_copy(&legend->param, &plot->param); // Copy legend configuration to the new legend plot
  legend->param.width = legend->param.height = 0.001; // Super small graph
  if(legend->param.legend_enabled == 0 && legend->param.info == 1) {
    printf("[save_legend] Force turning on legend_enabled flag\n");
  }
  legend->param.legend_enabled = 1;       // Forced to turn on
  plot_set_legend_pos(legend, "center");  // Hardcode legend pos
  plot_draw_axis(legend);                 // Create the super small figure
  int count = vec_count(plot->bar_types);
  if(count == 0) {
    error_exit("Current plot does not contain any bar type\n");
  }
  for(int i = 0;i < count;i++) {
    bar_type_t *type = vec_at(plot->bar_types, i);
    bar_t *bar = bar_init();
    // The bar should not be drawn
    bar->bottom = bar->height = bar->width = 0.0;
    bar->inited = 1;
    // Also duplicate the bar type and associate it with the bar
    plot_add_bar_type(legend, type->label, type->color, type->hatch);
    bar_set_type(bar, plot_find_bar_type(legend, type->label));
    // Directly call plot_draw_bar() without adding the bar to the plot
    plot_draw_bar(legend, bar);
    // No longer used
    bar_free(bar);
  }
  // Only call draw legend here without plotting other things
  plot_draw_legend(legend);
  if(mode == PLOT_SAVE_MODE_FILE) {
    plot_save_fig(legend, (const char *)arg);
  } else if(mode == PLOT_SAVE_MODE_BUF) {
    // Do not use buf_concat since this will free the second arg
    buf_append((buf_t *)arg, buf_c_str(legend->buf));
  } else {
    error_exit("Invalid save mode %d\n", mode);
  }
  plot_free(legend);
  return;
}

// This functions draws a color test bar graph
void plot_save_color_test_mode(plot_t *plot, int mode, void *arg) {
  plot_t *test = plot_init();
  // Use current plot's configuration
  plot_param_copy(&test->param, &plot->param);
  plot_draw_axis(test);
  plot_param_t *param = &test->param;
  // Force turn off legend
  param->legend_enabled = 0;
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
    bar->inited = 1;
    bar_set_type(bar, plot_find_bar_type(test, label_buf));
    // Print color code
    char color_buf[16];
    color_str_latex(bar_get_type(bar)->color, color_buf);
    bar_set_text(bar, color_buf);
    plot_draw_bar(test, bar);
    char xtick_text[16];
    snprintf(xtick_text, 16, "[%d]", i);
    plot_add_xtick(test, bar_pos + 0.5 * bar_width, xtick_text);
    bar_free(bar);
    bar_pos += bar_width;
  }
  plot_add_x_title(test, "Color Scheme Test");
  if(mode == PLOT_SAVE_MODE_FILE) {
    plot_save_fig(test, (const char *)arg);
  } else if(mode == PLOT_SAVE_MODE_BUF) {
    // Do not use buf_concat since this will free the second arg
    buf_append((buf_t *)arg, buf_c_str(test->buf));
  } else {
    error_exit("Invalid save mode %d\n", mode);
  }
  plot_free(test);
  return;
}

void plot_save_hatch_test_mode(plot_t *plot, int mode, void *arg) {
  plot_t *test = plot_init();
  // Use current plot's configuration
  plot_param_copy(&test->param, &plot->param);
  plot_param_t *param = &test->param;
  // Force turn off legend
  param->legend_enabled = 0;
  char label_buf[16];
  int usable = param->hatch_scheme->item_count - param->color_offset;
  double bar_width = 2.0; // To show the hatch we need fixed width bar
  param->width = usable * bar_width; // Graph width is extended as there are more hatches
  param->xlim_right = usable * bar_width; // Set X right limit to avoid blank
  double bar_height = param->height;
  double bar_pos = 0.0;
  // Must do it here since we adjusted the width
  plot_draw_axis(test);
  for(int i = param->color_offset;i < param->hatch_scheme->item_count;i++) {
    snprintf(label_buf, 16, "hatch %d", i);
    char hatch = param->hatch_scheme->base[i];
    plot_add_bar_type(test, label_buf, 0xFFFFFF, hatch);
    bar_t *bar = bar_init();
    bar->pos = bar_pos;
    bar->width = bar_width;
    bar->height = bar_height;
    bar->inited = 1;
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
    plot_draw_bar(test, bar);
    char xtick_text[16];
    snprintf(xtick_text, 16, "[%d]", i);
    plot_add_xtick(test, bar_pos + 0.5 * bar_width, xtick_text);
    bar_free(bar);
    bar_pos += bar_width;
  }
  plot_add_x_title(test, "Hatch Scheme Test");
  if(mode == PLOT_SAVE_MODE_FILE) {
    plot_save_fig(test, (const char *)arg);
  } else if(mode == PLOT_SAVE_MODE_BUF) {
    // Do not use buf_concat since this will free the second arg
    buf_append((buf_t *)arg, buf_c_str(test->buf));
  } else {
    error_exit("Invalid save mode %d\n", mode);
  }
  plot_free(test);
  return;
}

const char *plot_valid_legend_poses[] = {
  "right", "center left", "upper right", "lower right",
  "best", "center", "lower left", "center right", "upper left",
  "upper center", "lower center", NULL,
};

// Sets legend pos by copying the string to the param struct
// The given string should not be longer than the storage size
void plot_set_legend_pos(plot_t *plot, const char *pos) {
  int len = strlen(pos);
  if(len > PLOT_LEGEND_POS_MAX_SIZE - 1) {
    error_exit("Legend pos must be a string shorter than %d bytes (sees %d)\n", 
      PLOT_LEGEND_POS_MAX_SIZE, len);
  }
  const char **p = plot_valid_legend_poses;
  while(*p != NULL) {
    if(streq(*p, pos) == 1) {
      strcpy(plot->param.legend_pos, pos);
      return;
    }
    p++;
  }
  printf("Invalid legend pos. We support the following:\n");
  p = plot_valid_legend_poses;
  while(*p != NULL) {
    printf("\"%s\"%s ", *p, *(p + 1) == NULL ? "" : ",");
    p++;
  }
  putchar('\n');
  error_exit("Could not set legend pos: \"%s\"\n", pos);
  return;
}

void plot_set_legend_rows(plot_t *plot, int rows) {
  if(rows <= 0 && rows != -1) {
    error_exit("Legend rows must be > 0 or -1 for vertical legend (sees %d)\n", rows);
  }
  plot->param.legend_rows = rows;
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
  printf("[plot] Bar types %d\n", vec_count(plot->bar_types));
  if(verbose == 1) {
    for(int i = 0;i < vec_count(plot->bar_types);i++) {
      bar_type_t *type = (bar_type_t *)vec_at(plot->bar_types, i);
      bar_type_print(type);
    }
  }
  // Print current bar group
  if(plot->curr_bargrp != NULL) {
    printf("[plot] Current active bar group:\n");
    bargrp_print(plot->curr_bargrp, verbose);
  } else {
    printf("[plot] There is no currently active bar group\n");
  }
  // Print previously added bar groups
  int bar_count = 0;
  for(int i = 0;i < vec_count(plot->bargrps);i++) {
    bargrp_t *grp = vec_at(plot->bargrps, i);
    if(verbose == 1) {
      bargrp_print(grp, verbose);
    } 
    bar_count += bargrp_count(grp);
  }
  printf("[plot] Bar groups: size %d total bars %d\n", vec_count(plot->bargrps), bar_count);
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
  assert(len >= 0); // We support 0 length string
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
      char escaped_ch = parse_peek(parse);
      // We should take care that strings like "\\" should be handled as valid string
      // So we also skip over '\\' here. Other cases are fine, since they do not contain '\"' or '\\'
      if(escaped_ch == '\"' || escaped_ch == '\\') {
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
  if(ch == ';') return PARSE_ARG_NONE;
  else if(ch == '\"') return PARSE_ARG_STR;
  else if(ch == '_' || isalpha(ch)) return PARSE_ARG_IDENT;
  else if(ch == '.' || isdigit(ch)) return PARSE_ARG_NUM; // hex 0xfff dec .decimal oct 0777
  else if(ch == '@') return PARSE_ARG_FILE;
  else if(ch == '?') return PARSE_ARG_QMARK;
  char buf[16];
  parse_print_char(parse, ch, buf);
  error_exit("Unknown argument character: %s\n", buf);
  return -1;
}

// Read an argument in string form. The string is mapped to an integer using the given table
// The arg is mandatory. Report error and exit if not found or type error
int parse_get_mapped_arg(parse_t *parse, parse_mapped_arg_entry_t *table) {
  int next_arg = parse_next_arg(parse);
  if(next_arg == PARSE_ARG_STR) {
    char *s = parse_get_str(parse);
    while(table->key != NULL) {
      if(streq(table->key, s) == 1) {
        free(s);
        return table->value;
      }
      table++;
    }
    parse_report_pos(parse);
    error_exit("Invalid string argument: \"%s\"\n", s);
  } else {
    parse_report_pos(parse);
    error_exit("Invalid argument. Expecting a string.\n");
  }
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
// Empty string means placeholder - we still use the one from scheme, but we can specify the hatch without having to
// manually enter color
// If color overflows, we report error
void parse_cb_bar_type(parse_t *parse, plot_t *plot) {
  //(void)parse; (void)plot;
  //printf("entity bar type\n");
  int next_arg = parse_next_arg(parse);
  char *label = NULL;
  if(next_arg == PARSE_ARG_NONE) {
    parse_report_pos(parse);
    error_exit("Entity \"bar_type\" requires at least a label\n");
  } else if(next_arg == PARSE_ARG_QMARK) {
    parse_expect_char(parse, '?');
    printf("[+bar_type] Usage: +bar_type [label] [color] [hatch]\n");
    printf("[+bar_type] The mandatory label is a unique string giving an identifier of the type, which can be used to "
           "assign the type to a bar later.\n");
    printf("[+bar_type] Both color and hatch are optional. Ig not given, the color and/or hatch from the corresponding "
           "schemes will be used to initialize the type object. If given, they are of string type and must be "
           "following the color and hatch encoding. Empty strings will be ignored as placeholders\n");
    printf("[+bar_type] If default schemes are used, the corresponding offset will also be incremented. If either "
           "color or hatch overflows an error will be reported\n");
    return;
  }
  // Reading the label
  if(next_arg == PARSE_ARG_STR) {
    label = parse_get_str(parse);
  } else {
    parse_report_pos(parse);
    error_exit("Expecting a label for the bar group\n");
  }
  uint32_t color = -1U;
  char hatch = -1;
  if(parse_next_arg(parse) == PARSE_ARG_STR) {
    char *color_str = parse_get_str(parse);
    // If it is zero length string, we still use current color
    if(strlen(color_str) != 0) {
      color = color_decode(color_str);
      if(color == -1U) {
        parse_report_pos(parse);
        error_exit("Could not decode color string: \"%s\"\n", color_str);
      }
    }
    free(color_str);
  }
  if(parse_next_arg(parse) == PARSE_ARG_STR) {
    char *hatch_str = parse_get_str(parse);
    // If it is zero length string, we still use current color
    if(strlen(hatch_str) != 0) {
      hatch = hatch_decode(hatch_str);
      if(hatch == -1) {
        parse_report_pos(parse);
        error_exit("Could not decode hatch string: \"%s\"\n", hatch_str);
      }
    }
    free(hatch_str);
  }
  plot_param_t *param = &plot->param;
  if(color == -1U) {
    if(param->color_scheme == NULL) {
      parse_report_pos(parse);
      error_exit("Color scheme not specified\n");
    }
    assert(param->color_offset <= param->color_scheme->item_count);
    if(param->color_offset == param->color_scheme->item_count) {
      parse_report_pos(parse);
      error_exit("Color scheme \"%s\" overflows: offset %d\n", param->color_scheme->name, param->color_offset);
    }
    color = param->color_scheme->base[param->color_offset++];
  }
  if(hatch == -1) {
    if(param->hatch_scheme == NULL) {
      parse_report_pos(parse);
      error_exit("Hatch scheme not specified\n");
    }
    assert(param->hatch_offset <= param->hatch_scheme->item_count);
    if(param->hatch_offset == param->hatch_scheme->item_count) {
      parse_report_pos(parse);
      error_exit("Hatch scheme \"%s\" overflows: offset %d\n", param->hatch_scheme->name, param->hatch_offset);
    }
    hatch = param->hatch_scheme->base[param->hatch_offset++];
  }
  plot_add_bar_type(plot, label, color, hatch);
  if(parse_next_arg(parse) != PARSE_ARG_NONE) {
    parse_report_pos(parse);
    error_exit("Entity \"bar_type\" (label \"%s\") takes one mandatory argument and two optional arguments\n", label);
  }
  free(label);
  return;
}

// Bar group syntax:
// +bar_group "label" {
//
//  }
void parse_cb_bar_group(parse_t *parse, plot_t *plot) {
  //(void)parse; (void)plot;
  //printf("entity bar group\n");
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
  PARSE_GEN_PROP("legend_enabled", PARSE_LEGEND_ENABLED),
  PARSE_GEN_PROP("legend_rows", PARSE_LEGEND_ROWS),
  PARSE_GEN_PROP("legend_font_size", PARSE_LEGEND_FONT_SIZE),
  PARSE_GEN_PROP("legend_pos", PARSE_LEGEND_POS),
  // X Ticks
  PARSE_GEN_PROP("xtick_enabled", PARSE_XTICK_ENABLED),
  PARSE_GEN_PROP("xtick_length", PARSE_XTICK_LENGTH),
  PARSE_GEN_PROP("xtick_direction", PARSE_XTICK_DIRECTION),
  PARSE_GEN_PROP("xtick_font_size", PARSE_XTICK_FONT_SIZE),
  PARSE_GEN_PROP("xtick_rotation", PARSE_XTICK_ROTATION),
  PARSE_GEN_PROP("xtick_label_enabled", PARSE_XTICK_LABEL_ENABLED),
  // Y Ticks
  PARSE_GEN_PROP("ytick_enabled", PARSE_YTICK_ENABLED),
  PARSE_GEN_PROP("ytick_length", PARSE_YTICK_LENGTH),
  PARSE_GEN_PROP("ytick_direction", PARSE_YTICK_DIRECTION),
  PARSE_GEN_PROP("ytick_font_size", PARSE_YTICK_FONT_SIZE),
  PARSE_GEN_PROP("ytick_rotation", PARSE_YTICK_ROTATION),
  PARSE_GEN_PROP("ytick_label_enabled", PARSE_YTICK_LABEL_ENABLED),
  // X grid
  PARSE_GEN_PROP("xgrid_enabled", PARSE_XGRID_ENABLED),
  // Y grid
  PARSE_GEN_PROP("ygrid_enabled", PARSE_YGRID_ENABLED),
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
  // Bar group
  PARSE_GEN_PROP("bargrp_space", PARSE_BARGRP_SPACE),
  // py_t object
  PARSE_GEN_PROP("dry_run", PARSE_DRY_RUN),
  PARSE_GEN_PROP("info", PARSE_INFO),
};
const int parse_cb_top_props_count = sizeof(parse_cb_top_props) / sizeof(parse_cb_entry_t);

// Mapping axis direction string to integer
parse_mapped_arg_entry_t parse_mapped_arg_direction[] = {
  {"in", PLOT_DIRECTION_INSIDE},
  {"out", PLOT_DIRECTION_OUTSIDE},
  {"both", PLOT_DIRECTION_BOTH},
  PARSE_MAPPED_ARG_END,
};

parse_mapped_arg_entry_t parse_mapped_arg_dry_run[] = {
  {"disabled", PLOT_DRY_RUN_DISABLED},
  {"enabled", PLOT_DRY_RUN_ENABLED},
  {"show", PLOT_DRY_RUN_SHOW},
  PARSE_MAPPED_ARG_END,
};

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
        if(plot->param.info == 1) {
          printf("[parse] WARNING: The property \"fig_filename\" already exists, value \"%s\"\n", 
            plot->fig_filename);
        }
        free(plot->fig_filename);
      }
      plot->fig_filename = parse_get_str(parse);
    } break;
    case PARSE_LEGEND_FILENAME: {
      if(plot->legend_filename != NULL) {
        if(plot->param.info == 1) {
          printf("[parse] WARNING: The property \"legend_filename\" already exists, value \"%s\"\n", 
            plot->legend_filename);
        }
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
    // Legend
    case PARSE_LEGEND_ENABLED: {
      plot->param.legend_enabled = (int)parse_get_int64_range(parse, 0, 1);
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
    // X ticks
    case PARSE_XTICK_ENABLED: {
      plot->param.xtick_enabled = (int)parse_get_int64_range(parse, 0, 1);
    } break;
    case PARSE_XTICK_LENGTH: {
      plot->param.xtick_length = parse_get_double_range(parse, 0.0, PARSE_DOUBLE_MAX);
    } break;
    case PARSE_XTICK_DIRECTION: {
      int next_arg = parse_next_arg(parse);
      if(next_arg == PARSE_ARG_NUM) {
        plot->param.xtick_direction = (int)parse_get_int64_range(parse, 0, 2);
      } else if(next_arg == PARSE_ARG_STR) {
        plot->param.xtick_direction = parse_get_mapped_arg(parse, parse_mapped_arg_direction);
      }
    } break;
    case PARSE_XTICK_FONT_SIZE: {
      plot->param.xtick_font_size = (int)parse_get_int64_range(parse, 1, PARSE_INT64_MAX);
    } break;
    case PARSE_XTICK_ROTATION: {
      plot->param.xtick_rotation = (int)parse_get_int64_range(parse, 0, 359);
    } break;
    case PARSE_XTICK_LABEL_ENABLED: {
      plot->param.xtick_label_enabled = (int)parse_get_int64_range(parse, 0, 1);
    } break;
    // Y ticks
    case PARSE_YTICK_ENABLED: {
      plot->param.ytick_enabled = (int)parse_get_int64_range(parse, 0, 1);
    } break;
    case PARSE_YTICK_LENGTH: {
      plot->param.ytick_length = parse_get_double_range(parse, 0.0, PARSE_DOUBLE_MAX);
    } break;
    case PARSE_YTICK_DIRECTION: {
      int next_arg = parse_next_arg(parse);
      if(next_arg == PARSE_ARG_NUM) {
        plot->param.ytick_direction = (int)parse_get_int64_range(parse, 0, 2);
      } else if(next_arg == PARSE_ARG_STR) {
        plot->param.ytick_direction = parse_get_mapped_arg(parse, parse_mapped_arg_direction);
      }
    } break;
    case PARSE_YTICK_FONT_SIZE: {
      plot->param.ytick_font_size = (int)parse_get_int64_range(parse, 1, PARSE_INT64_MAX);
    } break;
    case PARSE_YTICK_ROTATION: {
      plot->param.ytick_rotation = (int)parse_get_int64_range(parse, 0, 359L);
    } break;
    case PARSE_YTICK_LABEL_ENABLED: {
      plot->param.ytick_label_enabled = (int)parse_get_int64_range(parse, 0, 1);
    } break;
    // X Grid
    case PARSE_XGRID_ENABLED: {
      plot->param.xgrid_enabled = (int)parse_get_int64_range(parse, 0, 1);
    } break;
    // Y Grid
    case PARSE_YGRID_ENABLED: {
      plot->param.ygrid_enabled = (int)parse_get_int64_range(parse, 0, 1);
    } break;
    // XY Title
    case PARSE_XTITLE_FONT_SIZE: {
      plot->param.xtitle_font_size = (int)parse_get_int64_range(parse, 1, PARSE_INT64_MAX);
    } break;
    case PARSE_YTITLE_FONT_SIZE: {
      plot->param.ytitle_font_size = (int)parse_get_int64_range(parse, 1, PARSE_INT64_MAX);
    } break;
    // Bar text
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
    // Limits
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
    // Bar group
    case PARSE_BARGRP_SPACE: {
      plot->param.bargrp_space = parse_get_double_range(parse, 0.0, PARSE_DOUBLE_MAX);
    } break;
    // Debug
    case PARSE_DRY_RUN: {
      int prev = plot->param.dry_run;
      int next_arg = parse_next_arg(parse);
      if(next_arg == PARSE_ARG_NUM) {
        plot->param.dry_run = (int)parse_get_int64_range(parse, 0, 2);
      } else if(next_arg == PARSE_ARG_STR) {
        plot->param.dry_run = parse_get_mapped_arg(parse, parse_mapped_arg_dry_run);
      }
      if(prev == PLOT_DRY_RUN_DISABLED && plot->param.dry_run != PLOT_DRY_RUN_DISABLED) {
        if(plot->param.info == 1) printf("[parse] Dry run mode enabled; Scripts will not be actually executed\n");
      } else if(prev != PLOT_DRY_RUN_DISABLED && plot->param.dry_run == PLOT_DRY_RUN_DISABLED) {
        if(plot->param.info == 1) printf("[parse] Dry run mode disabled; Scripts will be executed\n");
      }
    } break;
    case PARSE_INFO: {
      int next_arg = parse_next_arg(parse);
      if(next_arg == PARSE_ARG_NUM) {
        plot->param.dry_run = (int)parse_get_int64_range(parse, 0, 2);
      } else {
        char *info_str = parse_get_str(parse);
        if(streq(info_str, "disabled") == 1) {
          plot->param.info = 0;
        } else if(streq(info_str, "enabled") == 1) {
          plot->param.info = 1;
        } else {
          parse_report_pos(parse);
          error_exit("Invalid string value for property \"info\"\n");
        }
        free(info_str);
      }
    } break;
    default: {
      parse_report_pos(parse);
      error_exit("Peoperty \"%s\" cannot be assigned\n", name);
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
  PARSE_GEN_CB("set_hatch_scheme", parse_cb_set_hatch_scheme),
  PARSE_GEN_CB("set_color_scheme", parse_cb_set_color_scheme),
  PARSE_GEN_CB("test_hatch", parse_cb_test_hatch),
  PARSE_GEN_CB("test_color", parse_cb_test_color),
  PARSE_GEN_CB("dump", parse_cb_dump),
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

// Reads verbose flag as either numeric 0/1 value or "verbose string"
// If verbose flag is not found return 0 (default not verbose)
static int parse_get_verbose(parse_t *parse) {
  int next_arg = parse_next_arg(parse);
  int verbose = 0;
  if(next_arg == PARSE_ARG_IDENT) {
    char *verbose_ident = parse_get_ident(parse);
    if(streq(verbose_ident, "verbose") == 1) {
      verbose = 1;
    } else {
      parse_report_pos(parse);
      error_exit("Unknown verbose option for \"print\": \"%s\"\n", verbose_ident);
    }
    free(verbose_ident);
  } else if(next_arg == PARSE_ARG_NUM) {
    verbose = parse_get_int64_range(parse, 0, 1);
  }
  return verbose;
}

void parse_cb_print(parse_t *parse, plot_t *plot) {
  int next_arg = parse_next_arg(parse);
  if(next_arg == PARSE_ARG_NONE) {
    parse_report_pos(parse);
    error_exit("Function \"print\" expects at least 1 argument\n");
  } else if(next_arg == PARSE_ARG_QMARK) {
    parse_expect_char(parse, '?'); // Eat the symbol. The ';' will be processed by caller
    printf("[!print] Usage: !print [target] [\"verbose\"]/[arg]\n");
    printf("[!print] The following targets are supported:\n");
    printf("[!print]   plot, param, version, color, hatch, bar_type\n");
    printf("[!print] The following targets can have an optional \"verbose\" string arg\n");
    printf("[!print]   plot, param, version, color, hatch\n");
    printf("[!print] The following targets can have an optional numeric arg indicating only the element on the "
           "given index will be printed\n");
    printf("  bar_type\n");
    return;
  } else if(next_arg == PARSE_ARG_STR) {
    // Format string print
    char *fmt = parse_get_str(parse);
    buf_t *buf = buf_init();
    parse_print_fmt(parse, plot, buf, fmt);
    printf("%s", buf_c_str(buf));
    buf_free(buf);
    free(fmt);
    return;
  }
  plot_param_t *param = &plot->param;
  // First param of print function
  char *name = parse_get_ident(parse);
  int verbose = 0;
  next_arg = parse_next_arg(parse);
  if(streq(name, "plot") == 1) {
    verbose = parse_get_verbose(parse);
    plot_print(plot, verbose);
  } else if(streq(name, "param") == 1) {
    verbose = parse_get_verbose(parse);
    plot_param_print(param, verbose);
  } else if(streq(name, "version") == 1) {
    verbose = parse_get_verbose(parse);
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
  } else if(streq(name, "color") == 1) {
    verbose = parse_get_verbose(parse);
    if(param->color_scheme != NULL) {
      color_scheme_print(param->color_scheme, verbose);
    } else {
      printf("[parse] There is no color scheme to print\n");
    }
  } else if(streq(name, "hatch") == 1) {
    verbose = parse_get_verbose(parse);
    if(param->hatch_scheme != NULL) {
      hatch_scheme_print(param->hatch_scheme, verbose);
    } else {
      printf("[parse] There is no hatch scheme to print\n");
    }
  } else if(streq(name, "bar_type") == 1) {
    if(vec_count(plot->bar_types) == 0) {
      printf("[parse] There is no bar type objects to print\n");
    } else {
      next_arg = parse_next_arg(parse);
      char buf[16]; // Used to print color, used by all branches
      if(next_arg == PARSE_ARG_NONE) {
        // If no index then just print all of them
        for(int i = 0;i < vec_count(plot->bar_types);i++) {
          bar_type_t *type = vec_at(plot->bar_types, i);
          color_str(type->color, buf);
          printf("[bar_type] label \"%s\" color \"%s\" hatch \'%c\'\n", type->label, buf, type->hatch);
        }
      } else if(next_arg == PARSE_ARG_NUM) {
        // If there is an numeric index, print the one on that index
        int index = (int)parse_get_int64(parse);
        if(index < 0 || index >= vec_count(plot->bar_types)) {
          parse_report_pos(parse);
          error_exit("[parse] Bar type index out of range [0, %d) (see %d)\n", vec_count(plot->bar_types), index);
        }
        bar_type_t *type = vec_at(plot->bar_types, index);
        color_str(type->color, buf);
        printf("[bar_type] Index %d label \"%s\" color \"%s\" hatch \'%c\'\n", index, type->label, buf, type->hatch);
      } else if(next_arg == PARSE_ARG_STR) {
        // If there is a string, it will be treated as the label
        char *label = parse_get_str(parse);
        bar_type_t *type = plot_find_bar_type(plot, label);
        if(type == NULL) {
          parse_report_pos(parse);
          error_exit("Bar type label \"%s\" does not exist\n", label);
        }
        color_str(type->color, buf);
        printf("[bar_type] label \"%s\" color \"%s\" hatch \'%c\'\n", type->label, buf, type->hatch);
        free(label);
      }
    }
  } else {
    parse_report_pos(parse);
    error_exit("Unknown print target: \"%s\"\n", name);
  }
  // Check whether the state machine has deleped all arguments
  if(parse_next_arg(parse) != PARSE_ARG_NONE) {
    parse_report_pos(parse);
    error_exit("Unexpected argument for function \"print %s\"\n", name);
  }
  free(name);
  return;
}

void parse_cb_reset(parse_t *parse, plot_t *plot) {
  int next_arg = parse_next_arg(parse);
  if(next_arg == PARSE_ARG_NONE) {
    parse_report_pos(parse);
    error_exit("Function \"reset\" expects 1 argument\n");
  } else if(next_arg == PARSE_ARG_QMARK) {
    parse_expect_char(parse, '?');
    printf("[!reset] Usage: !reset [target]\n");
    printf("[!reset] Valid targets are: buf, param, plot\n");
    printf("[!reset] Existing color and hatch schemes are also freed when the plot or param is reset\n");
    return;
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
      if(plot->param.info == 1) {
        printf("[parse] Color scheme \"%s\" will be removed from plot during reset\n", param->color_scheme->name);
      }
    }
    if(param->hatch_scheme != NULL) {
      if(plot->param.info == 1) {
        printf("[parse] Hatch scheme \"%s\" will be removed from plot during reset\n", param->hatch_scheme->name);
      }
    }
    plot_param_copy(&plot->param, &default_param);
  }
  free(name);
  return;
}

void parse_cb_save_fig(parse_t *parse, plot_t *plot) {
  char *filename = NULL; // Given in arg list
  int next_arg = parse_next_arg(parse);
  if(next_arg == PARSE_ARG_QMARK) {
    parse_expect_char(parse, '?');
    printf("[!save_fig] Usage: !save_fig [file name]\n");
    printf("[!save_fig] The optional file name, if given, will override an existing file name. Otherwise, the existing file name "
           "will be used. If neither is present, error will be reported\n");
    return;
  }
  if(next_arg != PARSE_ARG_NONE) {
    filename = parse_get_str(parse);
    if(plot->fig_filename != NULL) {
      if(plot->param.info == 1) {
        printf("[parse] Overriding existing save fig filename: \"%s\"\n", plot->fig_filename);
      }
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
  char *filename = NULL;
  int next_arg = parse_next_arg(parse);
  if(next_arg == PARSE_ARG_QMARK) {
    parse_expect_char(parse, '?');
    printf("[!save_legend] Usage: !save_legend [file name]\n");
    printf("[!save_legend] The optional file name, if given, will override an existing file name. Otherwise, the existing file name "
           "will be used. If neither is present, error will be reported\n");
    return;
  }
  if(next_arg != PARSE_ARG_NONE) {
    filename = parse_get_str(parse);
    if(plot->legend_filename != NULL) {
      if(plot->param.info == 1) {
        printf("[parse] Overriding existing save legend filename: \"%s\"\n", plot->legend_filename);
      }
    }
    plot_save_legend_file(plot, filename);
    free(filename);
  } else {
    if(plot->legend_filename == NULL) {
      error_exit("Need a file name to save the legend (either as property or argument)\n");
    }
    plot_save_legend_file(plot, plot->legend_filename);
  }
  if(parse_next_arg(parse)) {
    error_exit("Function \"save_legend\" takes 1 optional argument\n");
  }
  return;
}

void parse_cb_set_hatch_scheme(parse_t *parse, plot_t *plot) {
  int next_type = parse_next_arg(parse);
  if(next_type == PARSE_ARG_NONE) {
    error_exit("Function \"set_hatch_scheme\" takes at least 1 argument\n");
  } else if(next_type == PARSE_ARG_QMARK) {
    parse_expect_char(parse, '?');
    printf("[!set_hatch_scheme] Usage: !set_hatch_scheme [name]/[file name]\n");
    printf("[!set_hatch_scheme] Name is a string for built-in schemes; File name is a file name indicator"
           " (beginning with '@'), which causes the scheme file being load\n");
    return;
  }
  plot_param_t *param = &plot->param;
  // Free current one if there is one
  if(param->hatch_scheme != NULL) {
    if(plot->param.info == 1) {
      printf("[parse] Overriding existing hatch scheme \"%s\"\n", param->hatch_scheme->name);
    }
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
  } else if(next_type == PARSE_ARG_QMARK) {
    parse_expect_char(parse, '?');
    printf("[!set_color_scheme] Usage: !set_color_scheme [name]/[file name]\n");
    printf("[!set_color_scheme] Name is a string for built-in schemes; File name is a file name indicator"
           " (beginning with '@'), which causes the scheme file being load\n");
    return;
  }
  plot_param_t *param = &plot->param;
  // Free current one if there is one
  if(param->color_scheme != NULL) {
    if(plot->param.info == 1) {
      printf("[parse] Overriding existing color scheme \"%s\"\n", param->color_scheme->name);
    }
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
  } else if(parse_next_arg(parse) == PARSE_ARG_QMARK) {
    parse_expect_char(parse, '?');
    printf("[!test_hatch] Usage: !test_hatch [file name]\n");
    printf("[!test_hatch] The optional file name, if given, will override an existing file name. Otherwise, the "
           "existing file name will be used. If neither is present, error will be reported\n");
    return;
  }
  char *filename = parse_get_str(parse);
  if(parse_next_arg(parse)) {
    error_exit("Function \"test_hatch\" only takes 1 argument\n");
  }
  if(plot->param.info == 1) {
    printf("[parse] Saving hatch test file to \"%s\"\n", filename);
  }
  plot_save_hatch_test_file(plot, filename);
  free(filename);
  return;
}

void parse_cb_test_color(parse_t *parse, plot_t *plot) {
  if(parse_next_arg(parse) == 0) {
    error_exit("Function \"test_color\" takes 1 argument\n");
  } else if(parse_next_arg(parse) == PARSE_ARG_QMARK) {
    parse_expect_char(parse, '?');
    printf("[!test_color] Usage: !test_color [file name]\n");
    printf("[!test_color] The optional file name, if given, will override an existing file name. Otherwise, the "
           "existing file name will be used. If neither is present, error will be reported\n");
    return;
  }
  char *filename = parse_get_str(parse);
  if(parse_next_arg(parse)) {
    error_exit("Function \"test_color\" only takes 1 argument\n");
  }
  if(plot->param.info == 1) {
    printf("[parse] Saving color test file to \"%s\"\n", filename);
  }
  plot_save_color_test_file(plot, filename);
  free(filename);
  return;
}

void parse_cb_dump(parse_t *parse, plot_t *plot) {
  int next_arg = parse_next_arg(parse);
  if(next_arg == PARSE_ARG_QMARK) {
    printf("[!dump] Usage: dump [target] [file name]\n");
    printf("[!dump] Valid targets are: plot, legend, color_test, hatch_test\n");
    printf("[!dump] The file name can be either a string or a file indicator\n");
    return;
  }
  char *ident = parse_get_ident(parse);
  char *filename = NULL;
  if(parse_next_arg(parse) == PARSE_ARG_FILE) {
    filename = parse_get_filename(parse);
  } else if(parse_next_arg(parse) == PARSE_ARG_STR) {
    filename = parse_get_str(parse);
  } else {
    parse_report_pos(parse);
    error_exit("Function \"dump\" expects a file name or file indicator as second argument\n");
  }
  buf_t *buf = NULL;
  int free_buf = 0;
  if(streq(ident, "plot") == 1) {
    plot_draw(plot);
    plot_reset_flags(plot); // Reset flags here
    buf = plot->buf;
    free_buf = 0;
  } else if(streq(ident, "legend") == 1) {
    buf = buf_init();
    free_buf = 1;
    plot_save_legend_buf(plot, buf);
  } else if(streq(ident, "color_test") == 1) {
    buf = buf_init();
    free_buf = 1;
    plot_save_color_test_buf(plot, buf);
  } else if(streq(ident, "hatch_test") == 1) {
    buf = buf_init();
    free_buf = 1;
    plot_save_hatch_test_buf(plot, buf);
  } else {
    parse_report_pos(parse);
    error_exit("Invalid dump target: \"%s\"\n", ident);
  }
  FILE *fp = fopen(filename, "w");
  SYSEXPECT(fp != NULL);
  // Write size - 1 bytes to avoid the terminating zero
  int wsize = buf_get_size(buf) - 1;
  int ret = fwrite(buf_c_str(buf), 1, wsize, fp);
  if(ret != wsize) {
    error_exit("Failed to write file \"%s\" (returned %d, expect %d)\n", filename, ret, wsize);
  } else {
    if(plot->param.info == 1) printf("[!dump] Successfully wrote %d bytes to file \"%s\"\n", wsize, filename);
  }
  fclose(fp);
  if(free_buf == 1) buf_free(buf);
  free(filename);
  free(ident);
  return;
}

static void parse_print_check_spec(parse_t *parse, const char *spec, char ch, const char *name) {
  const char *p = spec;
  while(*p != '\0') {
    if(*p == ch) return;
    p++;
  }
  parse_report_pos(parse);
  printf("Usable sprciciers: ");
  p = spec;
  while(*p != '\0') {
    printf("'%c' ", *p);
    p++;
  }
  putchar('\n');
  error_exit("Specifier \'%c\' could not be used to format property \"%s\"\n", ch, name);
  return;
}

// Whether the given char belongs to the string in spec
static int parse_print_is_spec(parse_t *parse, const char *spec, char ch) {
  (void)parse;
  while(*spec != '\0') {
    if(*spec == ch) return 1;
    spec++;
  }
  return 0;
}

// Prints the value of a property into the given buf
// This function uses the same format string as printf
void parse_print_prop(parse_t *parse, plot_t *plot, buf_t *buf, const char *name, const char *fmt) {
  parse_cb_entry_t cb_entry = parse_find_cb_entry(parse, parse_cb_top_props, parse_cb_top_props_count, name);
  int spec_len = strlen(fmt);
  assert(spec_len >= 2); // %?\0
  char spec_ch = fmt[spec_len - 1]; // Specifier character
  if(cb_entry.name == NULL) {
    parse_report_pos(parse);
    error_exit("Property name \"%s\" cannot be found for string formatting\n", name);
  }
  switch(cb_entry.prop) {
    case PARSE_XTITLE: {
      parse_print_check_spec(parse, PARSE_SPEC_STR, spec_ch, name);
      buf_printf(buf, fmt, plot->xtitle);
    } break;
    case PARSE_YTITLE: {
      parse_print_check_spec(parse, PARSE_SPEC_STR, spec_ch, name);
      buf_printf(buf, fmt, plot->ytitle);
    } break;
    case PARSE_FIG_FILENAME: {
      parse_print_check_spec(parse, PARSE_SPEC_STR, spec_ch, name);
      buf_printf(buf, fmt, plot->fig_filename);
    } break;
    case PARSE_LEGEND_FILENAME: {
      parse_print_check_spec(parse, PARSE_SPEC_STR, spec_ch, name);
      buf_printf(buf, fmt, plot->legend_filename);
    } break;
    case PARSE_WIDTH: {
      parse_print_check_spec(parse, PARSE_SPEC_FLOAT, spec_ch, name);
      buf_printf(buf, fmt, plot->param.width);
    } break;
    case PARSE_HEIGHT: {
      parse_print_check_spec(parse, PARSE_SPEC_FLOAT, spec_ch, name);
      buf_printf(buf, fmt, plot->param.height);
    } break;
    // Legend
    case PARSE_LEGEND_ENABLED: {
      parse_print_check_spec(parse, PARSE_SPEC_INT32, spec_ch, name);
      buf_printf(buf, fmt, plot->param.legend_enabled);
    } break;
    case PARSE_LEGEND_ROWS: {
      parse_print_check_spec(parse, PARSE_SPEC_INT32, spec_ch, name);
      buf_printf(buf, fmt, plot->param.legend_rows);
    } break;
    case PARSE_LEGEND_FONT_SIZE: {
      parse_print_check_spec(parse, PARSE_SPEC_INT32, spec_ch, name);
      buf_printf(buf, fmt, plot->param.legend_font_size);
    } break;
    case PARSE_LEGEND_POS: {
      parse_print_check_spec(parse, PARSE_SPEC_STR, spec_ch, name);
      buf_printf(buf, fmt, plot->param.legend_pos);
    } break;
    // X tick
    case PARSE_XTICK_ENABLED: {
      parse_print_check_spec(parse, PARSE_SPEC_INT32, spec_ch, name);
      buf_printf(buf, fmt, plot->param.xtick_enabled);
    } break;
    case PARSE_XTICK_LENGTH: {
      parse_print_check_spec(parse, PARSE_SPEC_FLOAT, spec_ch, name);
      buf_printf(buf, fmt, plot->param.xtick_length);
    } break;
    case PARSE_XTICK_DIRECTION: {
      parse_print_check_spec(parse, PARSE_SPEC_INT32, spec_ch, name);
      buf_printf(buf, fmt, plot->param.xtick_direction);
    } break;
    case PARSE_XTICK_FONT_SIZE: {
      parse_print_check_spec(parse, PARSE_SPEC_INT32, spec_ch, name);
      buf_printf(buf, fmt, plot->param.legend_font_size);
    } break;
    case PARSE_XTICK_ROTATION: {
      parse_print_check_spec(parse, PARSE_SPEC_INT32, spec_ch, name);
      buf_printf(buf, fmt, plot->param.xtick_rotation);
    } break;
    case PARSE_XTICK_LABEL_ENABLED: {
      parse_print_check_spec(parse, PARSE_SPEC_INT32, spec_ch, name);
      buf_printf(buf, fmt, plot->param.xtick_label_enabled);
    } break;
    // Y tick
    case PARSE_YTICK_ENABLED: {
      parse_print_check_spec(parse, PARSE_SPEC_INT32, spec_ch, name);
      buf_printf(buf, fmt, plot->param.ytick_enabled);
    } break;
    case PARSE_YTICK_LENGTH: {
      parse_print_check_spec(parse, PARSE_SPEC_FLOAT, spec_ch, name);
      buf_printf(buf, fmt, plot->param.ytick_length);
    } break;
    case PARSE_YTICK_DIRECTION: {
      parse_print_check_spec(parse, PARSE_SPEC_INT32, spec_ch, name);
      buf_printf(buf, fmt, plot->param.ytick_direction);
    } break;
    case PARSE_YTICK_FONT_SIZE: {
      parse_print_check_spec(parse, PARSE_SPEC_INT32, spec_ch, name);
      buf_printf(buf, fmt, plot->param.ytick_font_size);
    } break;
    case PARSE_YTICK_ROTATION: {
      parse_print_check_spec(parse, PARSE_SPEC_INT32, spec_ch, name);
      buf_printf(buf, fmt, plot->param.ytick_rotation);
    } break;
    case PARSE_YTICK_LABEL_ENABLED: {
      parse_print_check_spec(parse, PARSE_SPEC_INT32, spec_ch, name);
      buf_printf(buf, fmt, plot->param.ytick_label_enabled);
    } break;
    // X Grid
    case PARSE_XGRID_ENABLED: {
      parse_print_check_spec(parse, PARSE_SPEC_INT32, spec_ch, name);
      buf_printf(buf, fmt, plot->param.xgrid_enabled);
    } break;
    // Y Grid
    case PARSE_YGRID_ENABLED: {
      parse_print_check_spec(parse, PARSE_SPEC_INT32, spec_ch, name);
      buf_printf(buf, fmt, plot->param.ygrid_enabled);
    } break;
    // XY Title
    case PARSE_XTITLE_FONT_SIZE: {
      parse_print_check_spec(parse, PARSE_SPEC_INT32, spec_ch, name);
      buf_printf(buf, fmt, plot->param.xtitle_font_size);
    } break;
    case PARSE_YTITLE_FONT_SIZE: {
      parse_print_check_spec(parse, PARSE_SPEC_INT32, spec_ch, name);
      buf_printf(buf, fmt, plot->param.ytitle_font_size);
    } break;
    // Bar text
    case PARSE_BAR_TEXT_FONT_SIZE: {
      parse_print_check_spec(parse, PARSE_SPEC_INT32, spec_ch, name);
      buf_printf(buf, fmt, plot->param.bar_text_font_size);
    } break;
    case PARSE_BAR_TEXT_ROTATION: {
      parse_print_check_spec(parse, PARSE_SPEC_INT32, spec_ch, name);
      buf_printf(buf, fmt, plot->param.bar_text_rotation);
    } break;
    case PARSE_BAR_TEXT_DECIMALS: {
      parse_print_check_spec(parse, PARSE_SPEC_INT32, spec_ch, name);
      buf_printf(buf, fmt, plot->param.bar_text_decimals);
    } break;
    case PARSE_BAR_TEXT_RTRIM: {
     parse_print_check_spec(parse, PARSE_SPEC_INT32, spec_ch, name);
      buf_printf(buf, fmt, plot->param.bar_text_rtrim);
    } break;
    // Limits
    case PARSE_XLIM_LEFT: {
      parse_print_check_spec(parse, PARSE_SPEC_FLOAT, spec_ch, name);
      buf_printf(buf, fmt, plot->param.xlim_left);
    } break;
    case PARSE_XLIM_RIGHT: {
      parse_print_check_spec(parse, PARSE_SPEC_FLOAT, spec_ch, name);
      buf_printf(buf, fmt, plot->param.xlim_right);
    } break;
    case PARSE_YLIM_TOP: {
      parse_print_check_spec(parse, PARSE_SPEC_FLOAT, spec_ch, name);
      buf_printf(buf, fmt, plot->param.ylim_top);
    } break;
    case PARSE_YLIM_BOTTOM: {
      parse_print_check_spec(parse, PARSE_SPEC_FLOAT, spec_ch, name);
      buf_printf(buf, fmt, plot->param.ylim_bottom);
    } break;
    // Bar group
    case PARSE_BARGRP_SPACE: {
      parse_print_check_spec(parse, PARSE_SPEC_FLOAT, spec_ch, name);
      buf_printf(buf, fmt, plot->param.bargrp_space);
    } break;
    // Debug
    case PARSE_DRY_RUN: {
      parse_print_check_spec(parse, PARSE_SPEC_INT32, spec_ch, name);
      buf_printf(buf, fmt, plot->param.dry_run);
    } break;
    case PARSE_INFO: {
      parse_print_check_spec(parse, PARSE_SPEC_INT32, spec_ch, name);
      buf_printf(buf, fmt, plot->param.info);
    }
    default: {
      parse_report_pos(parse);
      error_exit("Property name \"%s\" cannot be used for string formatting\n", name);
    }
  }
  return;
}

// Prints a string, which can possibly be a format string containing format specifiers
// The parser will keep reading arguments for each specifier except %%
void parse_print_fmt(parse_t *parse, plot_t *plot, buf_t *buf, const char *str) {
  const char *p = str;
  int spec_index = 0;
  while(*p != '\0') {
    char ch = *p;
    if(ch == '%') {
      char ch2 = *(++p);
      if(ch2 == '\0') {
        parse_report_pos(parse);
        error_exit("Trailing '%%' at the end of string\n");
      } else if(ch2 == '%') {
        buf_putchar(buf, ch2); // %% means %
      } else {
        // p points to the first char after %
        const char *q = p;
        buf_t *fmt_buf = buf_init();
        buf_putchar(fmt_buf, '%');
        // Substring from '%' to the first letter
        char spec_ch;
        do {
          spec_ch = *q++; // q stops at the next char after the specifier
          buf_putchar(fmt_buf, spec_ch);
        } while(spec_ch != '\0' && parse_print_is_spec(parse, PARSE_SPEC_ALL, spec_ch) != 1);
        if(spec_ch == '\0') {
          parse_report_pos(parse);
          error_exit("Illegal format string: \"%s\"\n", p - 1);
        }
        char *fmt_str = buf_c_str(fmt_buf);  // Format string
        if(parse_next_arg(parse) == PARSE_ARG_NONE) {
          parse_report_pos(parse);
          error_exit("Format string \"%s\" (index %d) has no corresponding argument\n", p, spec_index);
        }
        char *name = parse_get_ident(parse); // Property name
        parse_print_prop(parse, plot, buf, name, fmt_str);
        free(name);
        buf_free(fmt_buf);
        // Need to -1 since we also have p++ below
        p = q - 1;
      }
      spec_index++;
    } else if(ch == '\\') {
      char ch2 = *(++p);
      switch(ch2) {
        case '\0': {
          parse_report_pos(parse);
          error_exit("Trailing '\\' at the end of string\n");
        } break;
        case 'n': ch2 = '\n'; break;
        case 't': ch2 = '\t'; break;
        case 'r': ch2 = '\r'; break;
        case 'v': ch2 = '\v'; break;
        case '\'': ch2 = '\''; break;
        case '\"': ch2 = '\"'; break;
        case '\\': ch2 = '\\'; break;
        default: {
          parse_report_pos(parse);
          char buf[16];
          parse_print_char(parse, ch2, buf);
          error_exit("Unknown escape character: %s\n", buf);
        } break;
      }
      buf_putchar(buf, ch2);
    } else {
      buf_putchar(buf, ch);
    }
    p++;
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
