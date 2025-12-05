#include <stdint.h>
#include <linux/netlink.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include "../common/common.h"
#include "netlink/attr.h"
#include "netlink/msg.h"
#include <netlink/netlink.h>
#include <netlink/handlers.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/socket.h>

struct radiotap_info {
	int8_t rssi;
	uint16_t channel_freq;
	uint16_t channel_flags;
};

struct wifi_frame_info {
	struct radiotap_info rt_info;
	uint8_t src_mac[6];
	uint8_t dst_mac[6];
	uint8_t bssid[6];
	uint8_t frame_t;
	uint8_t frame_st;
	uint16_t len;
};

static int frame_handler(struct nl_msg *msg, void *arg);
void handle_wifi_info(struct wifi_frame_info *info);

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

	while (1) {
		int ret = nl_recvmsgs_default(sock);
		if (ret) {
			fprintf(stderr, "Netlink message receive error (code:%d)\n", ret);
		}
	}

	nl_close(sock);
	nl_socket_free(sock);
}

static int frame_handler(struct nl_msg *msg, void *arg) {
	puts("Test");
	struct nlmsghdr *hdr = nlmsg_hdr(msg);
	struct genlmsghdr *ghdr = nlmsg_data(hdr);
	struct nlattr *attrs[WIFI_ATTR_MAX+1];
	nla_parse(attrs, WIFI_ATTR_MAX, genlmsg_attrdata(ghdr, 0), genlmsg_attrlen(ghdr, 0), NULL);

	struct wifi_frame_info *info = nla_data(attrs[WIFI_ATTR_FRAME]);
	handle_wifi_info(info);

	return NL_OK;
}

void print_mac(uint8_t mac[6]) {
	printf("%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void handle_wifi_info(struct wifi_frame_info *info) {
	printf("FROM ");
	print_mac(info->src_mac);
	printf(" TO ");
	print_mac(info->dst_mac);
	printf(" UNDER ");
	print_mac(info->bssid);
	puts("");
}


