
#include "primitives.h"

//* bar_t

bar_t *bar_init() {
  bar_t *bar = (bar_t *)malloc(sizeof(bar_t));
  SYSEXPECT(bar != NULL);
  memset(bar, 0x00, sizeof(bar_t));
  return bar;
}

