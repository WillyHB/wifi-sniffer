#include "ui.h"
#include "action_frame_parser.h"
#include "channel_hop.h"
#include "ctx.h"
#include "ft_parser.h"
#include "helpers.h"
#include <ncurses.h>
#include <string.h>
#include <time.h>

const uint8_t broadcast_mac[6] = {0xff,0xff,0xff,0xff,0xff,0xff};

void print_mac(WINDOW *win, struct ctx *ctx, uint8_t mac[6]) {
	int pair = -1;
	for (size_t i = 0; i < ctx->aps_count; i++) {
		if (memcmp(ctx->aps[i].mac, mac, 6) == 0) {
			pair = ctx->aps[i].col_pair+1;
			break;
		}
	}
	if (pair >= 0) wattron(win,COLOR_PAIR(pair));
	wprintw(win, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	if (pair >= 0) wattroff(win,COLOR_PAIR(pair));
}


void draw_ap_win(struct ctx *ctx) {
	for (size_t i = ctx->scroll, win_y = 1; i < ctx->aps_count && win_y < getmaxy(ctx->ap_win); i++, win_y++) {
		if (i == ctx->scroll) mvwprintw(ctx->ap_win,win_y,2, "* ");
		else mvwprintw(ctx->ap_win,win_y,2,"");
		wattron(ctx->ap_win, COLOR_PAIR(ctx->aps[i].col_pair+1));
		wprintw(ctx->ap_win, "[ ");
		for (int frq_i = 0; frq_i < ctx->aps[i].freq_num; frq_i++) {
			wprintw(ctx->ap_win, "%u ", ctx->aps[i].channel_freqs[frq_i]);
		}
		wprintw(ctx->ap_win, "] %-32s (", ctx->aps[i].ssid);
		print_mac(ctx->ap_win, ctx, ctx->aps[i].mac);
		wprintw(ctx->ap_win, ") ");
		wattroff(ctx->ap_win, COLOR_PAIR(ctx->aps[i].col_pair+1));

		if (ctx->aps[i].view == MAIN) {
			wprintw(ctx->ap_win, "Last Seen: %02ld sec ago [Beacons: %u] ", time(NULL) - ctx->aps[i].last_seen, ctx->aps[i].beacons);
		} else {
			wprintw(ctx->ap_win, "[TX: %u] [RX: %u] [Retries: %u]", 
					ctx->aps[i].tx_packets, ctx->aps[i].rx_packets, ctx->aps[i].retries);
		}

	}
	box(ctx->ap_win,0,0);
}

void draw_packet_win(struct ctx *ctx) {
	for (int i = 0; i < PACKET_BUF_SIZE; i++) {
		int index = (int)ctx->packet_buf_index - 1 - (int)i;
		if (index < 0) index += PACKET_BUF_SIZE;

		if (memcmp(&ctx->packet_buf[index], &(struct packet_info){0}, sizeof(struct packet_info)) == 0) {
				continue;
		}

		struct hdr_info hdr = ctx->packet_buf[index].hdr;

		mvwprintw(ctx->packet_win,i+1,2,""); 

		if (hdr.frame_t == T_MANAGEMENT) {
			wattron(ctx->packet_win, A_BOLD);
			wprintw(ctx->packet_win,"[%s] ", sub_frame_string(hdr.frame_t, hdr.frame_st));
			wattroff(ctx->packet_win, A_BOLD);
			if (hdr.frame_st == M_BEACON) {
				struct ap *ap = get_ap(ctx, hdr.bssid);
				wattron(ctx->packet_win, COLOR_PAIR(ap->col_pair+1));
				print_mac(ctx->packet_win, ctx, ap->mac);
				wprintw(ctx->packet_win," (%s)", ap->ssid);
				wattroff(ctx->packet_win, COLOR_PAIR(ap->col_pair+1));

			} else if ((hdr.frame_st == M_ACTION || hdr.frame_st == M_ACTION_NOACK)) {
				wprintw(ctx->packet_win, "CAT: %s CODE: %s", 
					action_cat_to_string(ctx->packet_buf[index].action.cat), 
					action_to_string(ctx->packet_buf[index].action.cat, ctx->packet_buf[index].action.code)); 
			} else if (hdr.frame_st == M_PROBE_RESPONSE) {
				struct ap *ap = get_ap(ctx, ctx->packet_buf[index].hdr.bssid);
				print_mac(ctx->packet_win, ctx, ctx->packet_buf[index].hdr.bssid);
				if (ap != NULL) wprintw(ctx->packet_win, "(%s)", ap->ssid);
			}
		} else if (memcmp(hdr.dst_mac, broadcast_mac, 6) == 0) {
			wattron(ctx->packet_win, A_BOLD);
			wprintw(ctx->packet_win, "[Broadcast] ");
			wattroff(ctx->packet_win, A_BOLD);
			print_mac(ctx->packet_win, ctx, hdr.dst_mac);
			wprintw(ctx->packet_win, " %s", sub_frame_string(hdr.frame_t, hdr.frame_st));
		} else {
			wattron(ctx->packet_win, A_BOLD);
			wprintw(ctx->packet_win, "[%s] ", sub_frame_string(hdr.frame_t, hdr.frame_st));
			wattroff(ctx->packet_win, A_BOLD);
			print_mac(ctx->packet_win, ctx, hdr.src_mac);
			wprintw(ctx->packet_win, " -> ");
			print_mac(ctx->packet_win, ctx, hdr.dst_mac);
			wprintw(ctx->packet_win, " (BSSID: ");
			print_mac(ctx->packet_win, ctx, hdr.bssid);
			wprintw(ctx->packet_win,") %s", frame_direction_to_string(hdr.frame_d)); 
		}

	}
	box(ctx->packet_win,0,0);
}

void draw_stats_win(struct ctx *ctx) {
	wattron(ctx->stats_win, A_BOLD);
	mvwprintw(ctx->stats_win, 1, 1, "Packets: %u", ctx->packets);
	mvwprintw(ctx->stats_win, 2, 1, "APs: %lu", ctx->aps_count);
	mvwprintw(ctx->stats_win, 3, 1, "Channel: %d (%dMHz)", current_channel(), channel_to_freq(current_channel()));
	/*
	struct ap *ap = NULL;
	for (int i = 0; i < ctx->aps_count; i++) {

	}
	*/
	wattroff(ctx->stats_win, A_BOLD);
	box(ctx->stats_win,0,0);
}

void draw_usage_win(struct ctx *ctx) {
	mvwprintw(ctx->usage_win,1,1, "(TAB) Toggle Channel Hop");
	mvwprintw(ctx->usage_win,2,1, "(SPACE) Pause Receiving");
	mvwprintw(ctx->usage_win,3,1, "(j/k) Scroll AP List");
	mvwprintw(ctx->usage_win,3,1, "(J/K) Top/Bottom AP List");
	mvwprintw(ctx->usage_win,4,1, "(h/l) Scroll AP Detail");
	mvwprintw(ctx->usage_win,5,1, "(H/L) Scroll ALL AP Detail");
	box(ctx->usage_win,0,0);
}

void draw_ui(struct ctx *ctx) {
	werase(ctx->stats_win);
	draw_stats_win(ctx);
	wnoutrefresh(ctx->stats_win);

	werase(ctx->packet_win);
	draw_packet_win(ctx);
	wnoutrefresh(ctx->packet_win);

	werase(ctx->ap_win);
	draw_ap_win(ctx);
	wnoutrefresh(ctx->ap_win);

	werase(ctx->usage_win);
	draw_usage_win(ctx);
	wnoutrefresh(ctx->usage_win);

	doupdate();
}

struct ctx *init_ui(void) {
	initscr();
	struct ctx *ctx = init_ctx();
	if (ctx == NULL) {
		return NULL;
	}

	noecho();
	cbreak();
	curs_set(0);
	start_color();
	nodelay(stdscr, 1);

	for (int i = 1; i < COLORS; i++) {
		init_pair(i, i, COLOR_BLACK);
	}

	return ctx;
}
