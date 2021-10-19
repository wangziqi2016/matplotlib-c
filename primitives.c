
#include "primitives.h"

//* bar_t

bar_t *bar_init() {
  bar_t *bar = (bar_t *)malloc(sizeof(bar_t));
  SYSEXPECT(bar != NULL);
  memset(bar, 0x00, sizeof(bar_t));
  return bar;
}

void bar_free(bar_t *bar) {
  free(bar);
  return;
}

void bar_set_tl(bar_t *bar, point_t tl) {
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