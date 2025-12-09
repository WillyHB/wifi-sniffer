#include "ctx.h"
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>

struct ctx *ctx_init() {
	struct ctx *ctx = malloc(sizeof(struct ctx));
	int h,w;
	getmaxyx(stdscr, h, w);
	ctx->ap_win = newwin(h/2, w-30, h/2, 0);
	ctx->packet_win = newwin(h/2, w-30, 0, 0);
	ctx->stats_win = newwin(h, 30, 0, w-30);

	return ctx;
}
void ctx_free(struct ctx *ctx) {

	delwin(ctx->ap_win);
	delwin(ctx->packet_win);
	delwin(ctx->stats_win);
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
