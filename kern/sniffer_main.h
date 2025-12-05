#ifndef SNIFFER_H
#define SNIFFER_H
#include "asm-generic/int-ll64.h"
#include "linux/ieee80211.h"
#include "linux/netdevice.h"
#include "linux/skbuff.h"
#include "net/ieee80211_radiotap.h"
#include "../common/common.h"


int mywifi_open(struct net_device *dev);
int mywifi_close(struct net_device *dev);
netdev_tx_t mywifi_xmit(struct sk_buff *skb, struct net_device *dev);
struct net_device_ops sniffer_ops = {
     .ndo_open         = mywifi_open,
     .ndo_stop         = mywifi_close,
     .ndo_start_xmit   = mywifi_xmit,
};


struct SNIFFER_PRIV {
	int dummy;
};

int sniffer_init(void);
void sniffer_clean(void);
void mywifi_dev_init(struct net_device *dev);
struct radiotap_info get_radiotap_info(struct ieee80211_radiotap_header *rt_hdr, u16 rt_len);
rx_handler_result_t mywifi_rx_handler(struct sk_buff **pskb); 
void set_mac(struct ieee80211_hdr *hdr, u8 tods, u8 fromds, u8 dst[6], u8 src[6], u8 bssid[6]);
#endif
