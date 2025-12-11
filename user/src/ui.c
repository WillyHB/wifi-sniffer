#include "ui.h"
#include "action_frame_parser.h"
#include "channel_hop.h"
#include "ctx.h"
#include "ft_parser.h"
#include "helpers.h"
#include "uthash.h"
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
	for (size_t i = ctx->scroll, win_y = 0; i < ctx->aps_count; i++, win_y++) {
		wattron(ctx->ap_win, COLOR_PAIR(ctx->aps[i].col_pair+1));
		mvwprintw(ctx->ap_win,win_y,2, "[ ");
		for (int frq_i = 0; frq_i < ctx->aps[i].freq_num; frq_i++) {
			wprintw(ctx->ap_win, "%u ", ctx->aps[i].channel_freqs[frq_i]);
		}
		wprintw(ctx->ap_win, "] %*s (", ctx->aps[i].ssid_len, ctx->aps[i].ssid);
		print_mac(ctx->ap_win, ctx, ctx->aps[i].mac);
		wprintw(ctx->ap_win, ")");
		wattroff(ctx->ap_win, COLOR_PAIR(ctx->aps[i].col_pair+1));

		wprintw(ctx->ap_win, "[TX: %u] [RX: %u] [Beacons: %u] [Last Seen: %ld sec ago]", 
				ctx->aps[i].tx_packets, ctx->aps[i].rx_packets, ctx->aps[i].beacons, time(NULL)- ctx->aps[i].last_seen);
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
	mvwprintw(ctx->usage_win,1,1, "(H) Toggle Channel Hop");
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
