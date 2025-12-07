#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include "../common/common.h"
#include "ft_parser.h"
#include "netlink/attr.h"

struct radiotap_info {
	int8_t rssi;
	uint16_t channel_freq;
	uint16_t channel_flags;
};

struct mgmt_body {
	uint8_t ies_len;
	uint8_t ies[];
};


struct data_body {

};

struct body_info {
	union {
		struct mgmt_body mgmt;
		struct data_body data;
	};
};

struct hdr_info {
	uint8_t src_mac[6];
	uint8_t dst_mac[6];
	uint8_t bssid[6];
	uint8_t frame_t;
	uint8_t frame_st;
	uint8_t retry;
	uint16_t len;
	FRAME_DIRECTION frame_d;
};

void handle_wifi_info(struct hdr_info *info, struct radiotap_info *rt_info, uint8_t *body, uint8_t body_len);
static int frame_handler(struct nl_msg *msg, void *arg);
uint8_t mgmt_fixed_params(MANAGEMENT_TYPE frame_st);
#endif
