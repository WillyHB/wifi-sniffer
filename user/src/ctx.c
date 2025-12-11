#include "ctx.h"
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>

struct ctx *init_ctx() {
	struct ctx *ctx = calloc(1,sizeof(struct ctx));
	if (ctx == NULL) {
		return NULL;
	}

	int h,w;
	getmaxyx(stdscr, h, w);
	ctx->ap_win = newwin(h/2, w-30, h/2, 0);
	ctx->packet_win = newwin(h/2, w-30, 0, 0);
	ctx->stats_win = newwin(h-10, 30, 0, w-30);
	ctx->usage_win = newwin(10, 30, h-10, w-30);
	return ctx;
}
void free_ctx(struct ctx *ctx) {
	delwin(ctx->ap_win);
	delwin(ctx->packet_win);
	delwin(ctx->stats_win);
	delwin(ctx->usage_win);
	endwin();

	free(ctx);
}

struct ap *get_ap(struct ctx *ctx, uint8_t mac[6]) {
	for (size_t i = 0; i < ctx->aps_count; i++) {
		if (memcmp(ctx->aps[i].mac, mac, 6) == 0) {
			return &ctx->aps[i];
		}
	}

	return NULL;
}
