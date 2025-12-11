#ifndef CTX_H
#define CTX_H

#include "channel_hop.h"
#include "helpers.h"
#include "../../common/common.h"
#include <ncurses.h>
#include <stdint.h>
#include <time.h>
#include "uthash.h"


struct ap {
	time_t last_seen;
	uint32_t rx_packets;
	uint32_t beacons;
	uint32_t tx_packets;
	uint8_t mac[6];
	uint8_t ssid[32];
	uint8_t ssid_len;
	short col_pair;
	uint16_t channel_freqs[CHANNELS_COUNT];
	uint8_t freq_num;
	UT_hash_handle hh;
};

struct action_frame {
	uint8_t cat;
	uint8_t code;
};

struct radiotap_info {
	int8_t rssi;
	uint16_t channel_freq;
	uint16_t channel_flags;
};

struct mgmt_body {
	uint8_t ies_len;
	uint8_t ies[];
};

struct hdr_info {
	uint8_t src_mac[6];
	uint8_t dst_mac[6];
	uint8_t bssid[6];
	uint8_t frame_t;
	uint8_t frame_st;
	uint8_t retry;
	uint16_t len;
	uint8_t tods;
	uint8_t fromds;
	FRAME_DIRECTION frame_d;
};
struct packet_info {
	union {
		struct action_frame action;
	};
	struct radiotap_info rt;
	struct hdr_info hdr;
};

struct ctx {
	uint8_t scroll;
	WINDOW *ap_win, *packet_win, *stats_win, *usage_win;
	struct ap aps[AP_CAPACITY];
	size_t aps_count;
	uint32_t packets;
	struct packet_info packet_buf[PACKET_BUF_SIZE];
	size_t packet_buf_index;
};
struct ctx *ctx_init();
void ctx_free(struct ctx*);
struct ap *get_ap(struct ctx *ctx, uint8_t mac[6]);
#endif
