#include <stddef.h>
#include <stdint.h>
#include <linux/netlink.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include "../../common/common.h"
#include "channel_hop.h"
#include "ctx.h"
#include "ft_parser.h"
#include "helpers.h"
#include "ie_iter.h"
#include "netlink/attr.h"
#include "netlink/errno.h"
#include "netlink/msg.h"
#include "ui.h"
#include <netlink/netlink.h>
#include <netlink/handlers.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/socket.h>
#include <linux/nl80211.h>
#include "main.h"

struct nl_sock *init_netlink_socket(struct ctx *ctx) {
	struct nl_sock *sock = nl_socket_alloc();
	if (sock == NULL) {
		fprintf(stderr, "Socket allocation failed\n");
		return NULL;
	}

	if (genl_connect(sock) < 0) {
		fprintf(stderr, "Socket connection failed\n");
		return NULL;
	}
	
	int mcgrp = genl_ctrl_resolve_grp(sock, WIFI_FAMILY_NAME, WIFI_MCGRP_NAME);
	if (mcgrp < 0) {
		fprintf(stderr, "Resolving netlink family failed\n");
		nl_socket_free(sock);
		return NULL;
	}

	if (nl_socket_add_membership(sock, mcgrp)) {
		fprintf(stderr, "Adding family membership to socket failed\n");
		return NULL;
	}
	

	if (nl_socket_modify_cb(sock, NL_CB_VALID, NL_CB_CUSTOM, frame_handler, ctx) < 0) {
		fprintf(stderr, "Socket callback modification failed\n");
		nl_socket_free(sock);
		return NULL;
	}

	nl_socket_disable_seq_check(sock);
	nl_socket_set_nonblocking(sock);

	return sock;
}

struct ctx *init_ui(void) {
	initscr();
	struct ctx *ctx = ctx_init();
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

int main() {

	struct ctx *ctx = init_ui();
	struct nl_sock *sock = init_netlink_socket(ctx) ;
	channel_init("mon0");

	uint8_t paused = 0;
	uint8_t hopping = 0;
	uint16_t hop_timer = 5000;
	while (1) {
		int c = getch();
		if (c == ' ') {
			if (!paused) mvprintw(0, 0,"PAUSED");
			refresh();
			paused = !paused;
		}else if (c == 'k'){
			if (ctx->scroll > 0) ctx->scroll--;
		} else if (c == 'j') {
			if (ctx->scroll < ctx->aps_count) ctx->scroll++;
		} else if (c == 'h') {
			hopping = !hopping;
		} else if (c == 'q') break;

		if (paused) continue;

		int ret = nl_recvmsgs_default(sock);
		
		if (ret < 0 && ret != -NLE_AGAIN) {
			continue;
			//fprintf(stderr, "Netlink message receive error (code:%d)\n", ret);
		}
		

		for (size_t i = 0; i < ctx->aps_count; i++) {
			if (time(NULL) - ctx->aps[i].last_seen > 30) {
				for (size_t j = i; j < ctx->aps_count-1; j++) {
					ctx->aps[j] = ctx->aps[j+1];
				}
				if (ctx->scroll >= ctx->aps_count) ctx->scroll--;

				ctx->aps_count--;
				i--;
			}
		}

		draw_ui(ctx);
		if (hopping && hop_timer-- <= 0) {
			hop_timer = 5000;
			next_channel("mon0");
		}
	}

	ctx_free(ctx);
	curs_set(1);

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
	int body_len = nla_len(attrs[WIFI_ATTR_BODY]);
	if (body_len < 0 || body == NULL || info == NULL || rt_info == NULL) return NL_OK;
	handle_wifi_info(arg, info, rt_info, body, body_len);

	return NL_OK;
}


void handle_wifi_info(struct ctx *ctx, struct hdr_info *info, struct radiotap_info *rt_info, uint8_t *body, size_t body_len) {
	if (info == NULL || rt_info == NULL || body == NULL) return;

	ctx->packet_buf[ctx->packet_buf_index] = (struct packet_info){.hdr = *info, .rt = *rt_info, .action = {0}};
	static short cur_col = 0;

	ctx->packets++;

	uint8_t ap_mac[6];
	memcpy(ap_mac, info->bssid, 6);

	struct ap *ap = get_ap(ctx,ap_mac);

	if (ap != NULL) {
		if (info->fromds) {
			ap->last_seen = time(NULL);
			ap->tx_packets++;
		}
		if (info->tods) ap->rx_packets++;
	}

	if (info->frame_t == T_MANAGEMENT) {
		if (info->frame_st == M_BEACON || info->frame_st == M_PROBE_RESPONSE) {
			if (ap == NULL) {
				ap = &ctx->aps[ctx->aps_count];
				memcpy(ap->mac, ap_mac, 6);
				size_t len = strlen("<hidden>");
				memcpy(ap->ssid, "<hidden>", len);
				ap->ssid[len] = 0;
				ap->ssid_len = strlen("<hidden>");
				ap->col_pair = cur_col+1;
				cur_col = (cur_col+1) % COLORS;
				ctx->aps_count++;
			}
		}

		if (info->frame_st == M_BEACON) {
			ap->last_seen = time(NULL);
			ap->beacons++;
			uint8_t flag = 0;
			for (int i = 0; i < ap->freq_num; i++) {
				if (ap->channel_freqs[i] == rt_info->channel_freq) flag = 1;
			}
			if (flag == 0) ap->channel_freqs[ap->freq_num++] = rt_info->channel_freq;

			iter_packet_ies(ap, &ctx->packet_buf[ctx->packet_buf_index], body, body_len);
			return;
		} else if (info->frame_st == M_ACTION || info->frame_st == M_ACTION_NOACK) {
			ctx->packet_buf[ctx->packet_buf_index].action.cat = body[0];
			ctx->packet_buf[ctx->packet_buf_index].action.code = body[1];
		}
	}


	ctx->packet_buf_index = (ctx->packet_buf_index+1) % PACKET_BUF_SIZE;
}

