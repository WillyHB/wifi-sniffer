#include "ui.h"
#include "action_frame_parser.h"
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

void draw_ui(struct ctx *ctx) {
	werase(ctx->ap_win);
	werase(ctx->packet_win);
	werase(ctx->stats_win);

	wattron(ctx->stats_win, A_BOLD);
	mvwprintw(ctx->stats_win, 1, 1, "Packets: %u", ctx->packets);
	mvwprintw(ctx->stats_win, 2, 1, "APs: %lu", ctx->aps_count);
	wattroff(ctx->stats_win, A_BOLD);

	for (size_t i = 0; i < ctx->aps_count; i++) {
		wattron(ctx->ap_win, COLOR_PAIR(ctx->aps[i].col_pair+1));
		mvwprintw(ctx->ap_win,i+1,2, "[ ");
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

	for (int i = 0; i < PACKET_BUF_SIZE; i++) {
		int index = (int)ctx->packet_buf_index - 1 - (int)i;
		if (index < 0) index += PACKET_BUF_SIZE;

		struct hdr_info hdr = ctx->packet_buf[index].hdr;

		mvwprintw(ctx->packet_win,i+1,2,""); 

		if (hdr.frame_t == T_MANAGEMENT) {
			if (hdr.frame_st == M_BEACON) return;
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
			}
		} else if (memcmp(hdr.dst_mac, broadcast_mac, 6) == 0) {
			wattron(ctx->packet_win, A_BOLD);
			wprintw(ctx->packet_win, "[Broadcast] ");
			wattroff(ctx->packet_win, A_BOLD);
			print_mac(ctx->packet_win, ctx, hdr.dst_mac);
			wprintw(ctx->packet_win, " %s", sub_frame_string(hdr.frame_t, hdr.frame_st));
		} else {
			wattron(ctx->packet_win, A_BOLD);
			wprintw(ctx->packet_win,"[%s] ", frame_direction_to_string(hdr.frame_d)); 
			wattroff(ctx->packet_win, A_BOLD);
			print_mac(ctx->packet_win, ctx, hdr.src_mac);
			wprintw(ctx->packet_win, " -> ");
			print_mac(ctx->packet_win, ctx, hdr.dst_mac);
			wprintw(ctx->packet_win, " (BSSID: ");
			print_mac(ctx->packet_win, ctx, hdr.bssid);
			wprintw(ctx->packet_win, ") %s", sub_frame_string(hdr.frame_t, hdr.frame_st));
		}

	}

	box(ctx->ap_win,0,0);
	box(ctx->packet_win,0,0);
	box(ctx->stats_win,0,0);
	refresh();
	wrefresh(ctx->ap_win);
	wrefresh(ctx->packet_win);
	wrefresh(ctx->stats_win);
}
