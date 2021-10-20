
#include "primitives.h"

//* String functions

char *strclone(const char *s) {
  int size = strlen(s) + 1;
  char *ret = (char *)malloc(size);
  SYSEXPECT(ret != NULL);
  memcpy(ret, s, size);
  return ret;
}

//* point_t

char *point_str(point_t point, int channel) {
  static char buf[POINT_CHANNEL_COUNT][POINT_CHANNEL_SIZE];
  assert(channel >= 0 && channel < POINT_CHANNEL_COUNT);
  snprintf(buf[channel], POINT_CHANNEL_SIZE, "<%.3lf, %.3lf>", point.x, point.y);
  return buf[channel];
}

//* bar_t

bar_t *bar_init() {
  bar_t *bar = (bar_t *)malloc(sizeof(bar_t));
  SYSEXPECT(bar != NULL);
  memset(bar, 0x00, sizeof(bar_t));
  return bar;
}

bar_t *bar_init_name(char *name) {
  bar_t *bar = bar_init();
  if(name != NULL) {
    bar->name = strclone(name);
  }
  return bar;
}

void bar_free(bar_t *bar) {
  if(bar->name != NULL) {
    free(bar->name);
  }
  free(bar);
  return;
}

void bar_set_error(const char *target, const char *existing1, const char *existing2) {
  error_exit("Could not set %s when %s and %s are both set\n", target, existing1, existing2);
}

void bar_set_tl(bar_t *bar, point_t tl) {
  // Check if tr or bl is set
  if((bar->pos_mask & BAR_POS_MASK_TR) && (bar->pos_mask & BAR_POS_MASK_WIDTH)) {
    
  }
  bar->tl = tl;
  bar->pos_mask |= BAR_POS_MASK_TL;
  return;
}

void bar_set_tr(bar_t *bar, point_t tr) {
  bar->tr = tr;
  bar->pos_mask |= BAR_POS_MASK_TR;
  return;
}

void bar_set_bl(bar_t *bar, point_t bl) {
  bar->bl = bl;
  bar->pos_mask |= BAR_POS_MASK_BL;
  return;
}

void bar_set_br(bar_t *bar, point_t br) {
  bar->br = br;
  bar->pos_mask |= BAR_POS_MASK_BR;
  return;
}

void bar_set_width(bar_t *bar, double width) {
  bar->width = width;
  bar->pos_mask |= BAR_POS_MASK_WIDTH;
  return;
}

void bar_set_height(bar_t *bar, double height) {
  bar->height = height;
  bar->pos_mask |= BAR_POS_MASK_HEIGHT;
  return;
}

//void bar_validate(bar_t *bar) {

//}

void bar_print(bar_t *bar) {
  printf("bar %s pos", (bar->name == NULL) ? "<NULL>" : bar->name);
  printf(" tl %s tr %s bl %s br %s",
    point_str(bar->tl, 0), point_str(bar->tr, 1), point_str(bar->bl, 2), point_str(bar->br, 3));
  printf(" width %.3lf height %.3lf", bar->width, bar->height);
  printf(" pos_mask 0x%X", bar->pos_mask);
  printf(" fill color 0x%06X edge color 0x%06X edge width %.3lf hatch \'%c'",
    bar->fill_color, bar->edge_color, bar->edge_width, bar->hatch);
  putchar('\n');
  return;
}
