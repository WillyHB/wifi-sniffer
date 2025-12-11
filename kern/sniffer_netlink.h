#ifndef SNIFFER_NETLINK_H
#define SNIFFER_NETLINK_H

#include "../common/common.h"
#include <net/genetlink.h>

struct radiotap_info {
	int8_t rssi;
	u16 channel_freq;
	u16 channel_flags;
};

struct hdr_info {
	u8 src_mac[6];
	u8 dst_mac[6];
	u8 bssid[6];
	u8 frame_t;
	u8 frame_st;
	u8 retry;
	u16 len; 
	u8 tods;
	u8 fromds;
	enum FRAME_DIRECTION frame_d;
};

void free_genl_family(void);
void init_genl_family(void);
void send_to_userspace(struct hdr_info *hdr_info, struct radiotap_info *rt_info, u8 body_len, u8 *body);
#endif

