#include <stdint.h>
#include <linux/netlink.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include "../common/common.h"
#include "ft_parser.h"
#include "ie_iter.h"
#include "netlink/attr.h"
#include "netlink/msg.h"
#include <netlink/netlink.h>
#include <netlink/handlers.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/socket.h>
#include "main.h"

#include <ncurses.h>
#include <panel.h>

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
	
	/*
	int family = genl_ctrl_resolve(sock, WIFI_FAMILY_NAME);
	if (family < 0) {
		fprintf(stderr, "Resolving netlink family failed\n");
		nl_socket_free(sock);
		return 1;
	}
	*/

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

	//WINDOW *win = initscr();

	while (1) {
		int ret = nl_recvmsgs_default(sock);
		if (ret) {
			fprintf(stderr, "Netlink message receive error (code:%d)\n", ret);
		}
	}

	//endwin();

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

void print_mac(uint8_t mac[6]) {
	printf("%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void handle_wifi_info(struct hdr_info *info, struct radiotap_info *rt_info, uint8_t *body, uint8_t body_len) {
	if (info == NULL) return;

	printf("[%s] %s", frame_direction_to_string(info->frame_d), sub_frame_string(info->frame_t, info->frame_st));
	if (info->frame_t == T_MANAGEMENT) {
		struct ie_iterator *iter = ie_iter(body, body_len, info->frame_st);

		while (ie_iter_next(iter)) {
			if (iter->cur_id == 0) {
				printf("Found SSID\n");
			}
		}

		ie_iter_free(iter);
	}

	printf(" FROM ");
	print_mac(info->src_mac);
	printf(" TO ");
	print_mac(info->dst_mac);
	printf(" BSSID ");
	print_mac(info->bssid);
	printf(" RSSI "); // 0 cause of hardware limitations :(
	printf("%d", rt_info->rssi);
	printf(" FREQ ");
	printf("%d\n", rt_info->channel_freq);
}

uint8_t mgmt_fixed_params(MANAGEMENT_TYPE frame_st) {
	switch (frame_st) {
		case M_BEACON:
		case M_TIMING_ADVERTISEMENT:
		case M_PROBE_RESPONSE: return 12;
		case M_RESERVED:
		case M_ACTION:
		case M_DISASSOCIATION:
		case M_DEAUTHENTICATION: return 2;
		case M_PROBE_REQUEST: return 0;
		case M_ASSOCIATION_REQUEST: return 4;
		case M_AUTHENTICATION: 
		case M_ASSOCIATION_RESPONSE:
		case M_REASSOCIATION_REQUEST:
		case M_REASSOCIATION_RESPONSE: return 6;
	}
}

void parse_mgmt_body(uint8_t *body, uint8_t body_len, MANAGEMENT_TYPE frame_st, struct mgmt_body *info) {
}
