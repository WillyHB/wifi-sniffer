#include <stddef.h>
#include <stdint.h>
#include <linux/netlink.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include "../common/common.h"
#include "ft_parser.h"
#include "ie_iter.h"
#include "netlink/attr.h"
#include "netlink/errno.h"
#include "netlink/msg.h"
#include <netlink/netlink.h>
#include <netlink/handlers.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/socket.h>
#include "main.h"

#include <ncurses.h>

struct ap {
	time_t last_seen;
	uint32_t rx_packets;
	uint32_t beacons;
	uint32_t tx_packets;
	uint8_t mac[6];
	uint8_t ssid[32];
	uint8_t ssid_len;
	short col_pair;
	uint16_t channel_freq;
};

short cur_col = 0;

struct ap aps[128] = {0};
size_t aps_count = 0;

uint32_t packets = 0;

WINDOW *ap_win;
WINDOW *packet_win;

#define PACKET_BUF_SIZE 20

struct action_frame {
	uint8_t cat;
	uint8_t code;
};

struct packet_info {
	struct radiotap_info rt;
	struct hdr_info hdr;
	union {
		struct action_frame action;
	};
};

struct packet_info packet_buf[PACKET_BUF_SIZE] = {0};
size_t packet_buf_index;

void print_mac(WINDOW *win, uint8_t mac[6]) {
	int pair = -1;
	for (size_t i = 0; i < aps_count; i++) {
		if (memcmp(aps[i].mac, mac, 6) == 0) {
			pair = aps[i].col_pair+1;
			break;
		}
	}
	if (pair >= 0) wattron(win,COLOR_PAIR(pair));
	wprintw(win, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	if (pair >= 0) wattroff(win,COLOR_PAIR(pair));
}

struct ap *get_ap(uint8_t mac[6]) {
	for (size_t i = 0; i < aps_count; i++) {
		if (memcmp(aps[i].mac, mac, 6) == 0) {
			return &aps[i];
		}
	}

	return NULL;
}

void draw_ui() {
	werase(ap_win);
	werase(packet_win);

	for (size_t i = 0; i < aps_count; i++) {
		wattron(ap_win, COLOR_PAIR(aps[i].col_pair+1));
		mvwprintw(ap_win,i+1, 2, 
				"[%dMHz] %*s (", aps[i].channel_freq, aps[i].ssid_len, aps[i].ssid);
		print_mac(ap_win, aps[i].mac);
		wprintw(ap_win, ")");
		wattroff(ap_win, COLOR_PAIR(aps[i].col_pair+1));

		wprintw(ap_win, "[TX] %u [RX] %u [Beacons] %u [Last Seen] %ld sec ago", 
				aps[i].tx_packets, aps[i].rx_packets, aps[i].beacons, time(NULL)-aps[i].last_seen);
	}

	wattron(packet_win, A_BOLD);
	mvwprintw(packet_win, 1, 2, "Total Packets: %u", packets);
	wattroff(packet_win, A_BOLD);

	for (int i = 0; i < PACKET_BUF_SIZE; i++) {
		int index = (int)packet_buf_index - 1 - (int)i;
		if (index < 0) index += PACKET_BUF_SIZE;

		struct hdr_info hdr = packet_buf[index].hdr;

		if (hdr.frame_t == T_MANAGEMENT) {
			wattron(packet_win, A_BOLD);
			mvwprintw(packet_win,i+3,2,"[%s] ", sub_frame_string(hdr.frame_t, hdr.frame_st));
			wattroff(packet_win, A_BOLD);
			if (hdr.frame_st == M_BEACON) {
				struct ap *ap = get_ap(hdr.bssid);
				wattron(packet_win, COLOR_PAIR(ap->col_pair+1));
				print_mac(packet_win, ap->mac);
				wprintw(packet_win," (%s)", ap->ssid);
				wattroff(packet_win, COLOR_PAIR(ap->col_pair+1));

			} else if ((hdr.frame_st == M_ACTION || hdr.frame_st == M_ACTION_NOACK)) {
				wprintw(packet_win, "CAT: %s CODE: %d", 
					action_cat_to_string(packet_buf[index].action.cat), packet_buf[index].action.code); 
			}
		} else {
			wattron(packet_win, A_BOLD);
			mvwprintw(packet_win,i+3,2,"[%s] ", frame_direction_to_string(hdr.frame_d)); 
			wattroff(packet_win, A_BOLD);
			print_mac(packet_win, hdr.src_mac);
			wprintw(packet_win, " -> ");
			print_mac(packet_win, hdr.dst_mac);
			wprintw(packet_win, " (BSSID: ");
			print_mac(packet_win, hdr.bssid);
			wprintw(packet_win, ") %s", sub_frame_string(hdr.frame_t, hdr.frame_st));
		}

	}

	box(ap_win,0,0);
	box(packet_win,0,0);
	wrefresh(ap_win);
	wrefresh(packet_win);
}

int main() {
	struct nl_sock *sock = nl_socket_alloc();
	if (sock == NULL) {
		fprintf(stderr, "Socket allocation failed\n");
		return 1;
	}

	if (genl_connect(sock) < 0) {
		fprintf(stderr, "Socket connection failed\n");
		return 1;
	}
	
	int mcgrp = genl_ctrl_resolve_grp(sock, WIFI_FAMILY_NAME, WIFI_MCGRP_NAME);
	if (mcgrp < 0) {
		fprintf(stderr, "Resolving netlink family failed\n");
		nl_socket_free(sock);
		return 1;
	}

	if (nl_socket_add_membership(sock, mcgrp)) {
		fprintf(stderr, "Adding family membership to socket failed\n");
		return 1;
	}
	
	if (nl_socket_modify_cb(sock, NL_CB_VALID, NL_CB_CUSTOM, frame_handler, NULL) < 0) {
		fprintf(stderr, "Socket callback modification failed\n");
		nl_socket_free(sock);
		return 1;
	}

	nl_socket_disable_seq_check(sock);
	nl_socket_set_nonblocking(sock);

	initscr();

	int h,w;
	getmaxyx(stdscr, h, w);

	ap_win = newwin(h/2, w, h/2, 0);
	packet_win = newwin(h/2, w, 0, 0);

	noecho();
	cbreak();
	curs_set(0);
	start_color();
	nodelay(stdscr, 1);

	for (int i = 1; i < COLORS; i++) {
		init_pair(i, i, COLOR_BLACK);
	}

	int paused = 0;

	while (1) {
		int c = getch();
		if (c == ' ') {
			mvprintw((h/2), (w/2),"PAUSED");
			refresh();
			paused = !paused;
		}

		else if (c == 'q') break;

		if (paused) continue;

		int ret = nl_recvmsgs_default(sock);
		if (ret < 0 && ret != -NLE_AGAIN) {
			fprintf(stderr, "Netlink message receive error (code:%d)\n", ret);
		}

		for (size_t i = 0; i < aps_count; i++) {
			if (time(NULL) - aps[i].last_seen > 30) {
				for (size_t j = i; j < aps_count-1; j++) {
					aps[j] = aps[j+1];
				}

				aps_count--;
			}
		}
		
		draw_ui();
	}

	/*
	for (int i = 1; i < COLORS; i++) {
		free_pair(i);
	}
	*/

	curs_set(1);
	delwin(ap_win);
	delwin(packet_win);
	endwin();

	nl_close(sock);
	nl_socket_free(sock);
}

static int frame_handler(struct nl_msg *msg, void *arg) {
	struct nlmsghdr *hdr = nlmsg_hdr(msg);
	struct genlmsghdr *ghdr = nlmsg_data(hdr);
	struct nlattr *attrs[WIFI_ATTR_MAX+1];
	nla_parse(attrs, WIFI_ATTR_MAX, genlmsg_attrdata(ghdr, 0), genlmsg_attrlen(ghdr, 0), NULL);

	struct hdr_info *info = nla_data(attrs[WIFI_ATTR_HEADER]);
	struct radiotap_info *rt_info = nla_data(attrs[WIFI_ATTR_RADIOTAP]);
	uint8_t *body = nla_data(attrs[WIFI_ATTR_BODY]);
	uint8_t body_len = nla_len(attrs[WIFI_ATTR_BODY]);
	handle_wifi_info(info, rt_info, body, body_len);

	return NL_OK;
}


void handle_wifi_info(struct hdr_info *info, struct radiotap_info *rt_info, uint8_t *body, uint8_t body_len) {
	if (info == NULL || rt_info == NULL || body == NULL) return;
	packet_buf[packet_buf_index] = (struct packet_info){.hdr = *info, .rt = *rt_info, .action = {0}};

	packets++;

	uint8_t ap_mac[6];
	memcpy(ap_mac, info->bssid, 6);

	struct ap *ap = get_ap(ap_mac);
	if (ap == NULL) {
		ap = &aps[aps_count];
		memcpy(ap->mac, ap_mac, 6);
		size_t len = strlen("<hidden>");
		memcpy(ap->ssid, "<hidden>", len);
		ap->ssid[len] = 0;
		ap->ssid_len = strlen("<hidden>");
		ap->col_pair = cur_col+1;
		cur_col = (cur_col+1) % COLORS;
		aps_count++;
	} 


	if (info->fromds) {
		ap->last_seen = time(NULL);
		ap->tx_packets++;
	}
	if (info->tods) ap->rx_packets++;


	if (info->frame_t == T_MANAGEMENT) {
		struct ie_iterator *iter = ie_iter(body, body_len, info->frame_st);
		if (info->frame_st == M_BEACON) {
			ap->last_seen = time(NULL);
			ap->beacons++;
			ap->channel_freq = rt_info->channel_freq;
		}

		if (info->frame_st == M_ACTION || info->frame_st == M_ACTION_NOACK) {
			packet_buf[packet_buf_index].action.cat = body[0];
			packet_buf[packet_buf_index].action.code = body[1];
		}

		while (!ie_iter_next(iter)) {
			if (iter->cur_id == 0) {
				if (memcmp(ap->ssid, iter->cur, iter->cur_len) != 0) {
					memcpy(ap->ssid, iter->cur, iter->cur_len);
					ap->ssid[iter->cur_len] = 0;
					ap->ssid_len = iter->cur_len;
				}
			}
		}

		ie_iter_free(iter);
	}

	packet_buf_index = (packet_buf_index+1) % PACKET_BUF_SIZE;
}

