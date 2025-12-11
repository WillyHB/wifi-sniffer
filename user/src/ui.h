#ifndef UI_H
#define UI_H

#include <ncurses.h>
#include "ctx.h"

void draw_ui(struct ctx *ctx);
struct ctx *init_ui(void);
#endif
