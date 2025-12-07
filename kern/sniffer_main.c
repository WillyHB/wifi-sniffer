#include <linux/module.h>
#include <linux/byteorder/generic.h>
#include <linux/ip.h>
#include <linux/printk.h>
#include <linux/rtnetlink.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/tcp.h>
#include <net/ieee80211_radiotap.h>
#include <linux/ieee80211.h>
#include <net/cfg80211.h>
#include <net/genetlink.h>
#include <net/netlink.h>
#include "sniffer_main.h"
#include "sniffer_netlink.h"

// Good to remember:
// STA: Station which participates in an 802.11 network (laptop, phone, even AP)
// BSS: Group of STAs which can talk to each other under a single AP
// BSSID: The unique MAC Address which identifies a BSS (Usually the APs MAC address)
// IBSS: Independent BSS (So no AP, only STAs talking to each other)
// DS: Distribution System - Infrastructure which connects APs (Ethernet or other)

struct net_device *mydev;
struct net_device *real_dev;

void mywifi_dev_init(struct net_device *dev) {
	if (dev == NULL) return;

	dev->netdev_ops = &sniffer_ops;

	ether_setup(dev);
	eth_random_addr((u8*)dev->dev_addr);
	dev->mtu = 1500; // Max Transition Unit (largest payload)
    dev->tx_queue_len = 100;
    dev->flags = IFF_NOARP | IFF_BROADCAST | IFF_MULTICAST;
	dev->addr_assign_type = NET_ADDR_RANDOM;
}

int sniffer_init(void) {
	printk(KERN_INFO "Sniffer loaded!\n");

	/*
	mydev = alloc_netdev(
		sizeof(struct SNIFFER_PRIV), 
		"mywifi%d", 
		NET_NAME_ENUM, 
		mywifi_dev_init);

	if (!mydev) {
		printk(KERN_ERR "Failed to allocate net_device\n");
		return ENOMEM;
	}

	if(register_netdev(mydev)) {
      	printk(KERN_ERR "Failed to register net device\n");
        free_netdev(mydev);
		return -1;
	}
	*/

	real_dev = dev_get_by_name(&init_net, "mon0");

	if (!real_dev) {
		printk(KERN_ERR "Net Device %s not found", "mon0");
		//unregister_netdevice(mydev);
		//free_netdev(mydev);
		return -1;
	}

	rtnl_lock();
	int ret = netdev_rx_handler_register(real_dev, mywifi_rx_handler, /*mydev*/ NULL);
	rtnl_unlock();

	if (ret) {
		printk(KERN_ERR "Failed to register RX handler\n");
		dev_put(real_dev);
		//unregister_netdev(mydev);
		//free_netdev(mydev);
		return ret;
	}

	init_genl_family();
    printk(KERN_INFO "Netdevice registered successfully\n");
	return 0;
}

struct radiotap_info get_radiotap_info(struct ieee80211_radiotap_header *rt_hdr, u16 rt_len) {
	struct ieee80211_radiotap_iterator iter = {0};
	int ret = ieee80211_radiotap_iterator_init(&iter , rt_hdr , rt_len , NULL);
	if (ret) {
		pr_err("Radiotap iterator init failed\n");
		return (struct radiotap_info){0};
	}

	struct radiotap_info info = {0};

	while (!ret) {
		ret = ieee80211_radiotap_iterator_next(&iter);
		if (ret) continue;
		if (iter.this_arg == NULL) continue;

		pr_info("Arg index: %d\n", iter.this_arg_index);

		switch (iter.this_arg_index) {
			case IEEE80211_RADIOTAP_DB_ANTSIGNAL:
			case 20:
				info.rssi = *(int8_t*)(iter.this_arg);
				printk(KERN_INFO "Test, here! :) %d\n", *(int8_t*)(iter.this_arg));
				break;
			case IEEE80211_RADIOTAP_CHANNEL:
				info.channel_freq = get_unaligned_le16(iter.this_arg);
				info.channel_flags = get_unaligned_le16(iter.this_arg + 2);
				break;
		}
	}

	return info;
}

enum FRAME_DIRECTION parse_fromtods(struct ieee80211_hdr *hdr, u8 tods, u8 fromds, u8 dst[6], u8 src[6], u8 bssid[6]) {
	if (tods == 0 && fromds == 0) { // Not going or coming from DS, so from a non AP STA
		ether_addr_copy(dst, hdr->addr1);
		ether_addr_copy(src, hdr->addr2);
		ether_addr_copy(bssid, hdr->addr3);
		return CLIENT_TO_CLIENT;
	} else if (tods == 1 && fromds == 0) { // Data frame sent to DS by an STA through an AP
		ether_addr_copy(dst, hdr->addr1);
		ether_addr_copy(bssid, hdr->addr2);
		ether_addr_copy(src, hdr->addr3);
		return CLIENT_TO_AP;
	} else if (tods == 0 && fromds == 1) { // Data frame exiting DS through AP to an STA
		ether_addr_copy(bssid, hdr->addr1);
		ether_addr_copy(src, hdr->addr2);
		ether_addr_copy(dst, hdr->addr3);
		return AP_TO_CLIENT;
	} else if (tods == 1 && fromds == 1) { // STA forwards traffic - AP-to-AP traffic, wireless bridging
		ether_addr_copy(dst, hdr->addr3);
		ether_addr_copy(src, hdr->addr4);
		return AP_TO_AP;
	}

	return 0;
}

rx_handler_result_t mywifi_rx_handler(struct sk_buff **pskb) {
	struct sk_buff *skb = *pskb;

	struct hdr_info hdr_info;
	struct radiotap_info rt_info;
	struct body_info b_info;

	struct ieee80211_radiotap_header *rt_hdr;
	struct ieee80211_hdr *hdr;
	u16 fc;
	u16 hdr_len;
	u16 rt_len;

	rt_hdr = (struct ieee80211_radiotap_header*)skb->data;
	rt_len = le16_to_cpu(rt_hdr->it_len);
	if (skb->len < rt_len)
		return RX_HANDLER_PASS;
	rt_info = get_radiotap_info(rt_hdr, rt_len);

	hdr = (struct ieee80211_hdr*)(skb->data+rt_len);
	fc = le16_to_cpu(hdr->frame_control);
	hdr_len = le16_to_cpu(ieee80211_hdrlen(fc));

	hdr_info.frame_t = (fc & IEEE80211_FCTL_FTYPE) >> 2;
	hdr_info.frame_st = (fc & IEEE80211_FCTL_STYPE) >> 4;
	hdr_info.retry = (fc & IEEE80211_FCTL_RETRY) >> 11;
	u8 tods = (fc & IEEE80211_FCTL_TODS) >> 8;
	u8 fromds = (fc & IEEE80211_FCTL_FROMDS) >> 9;
	hdr_info.frame_d = parse_fromtods(hdr, tods, fromds, hdr_info.dst_mac, hdr_info.src_mac, hdr_info.bssid);

	u8 *body = skb->data + rt_len + hdr_len;
	u8 body_len = skb->len - rt_len - hdr_len;

	if (body_len >= 4) { // strip NCS, not sure if it's always present...
		body_len -= 4;
	}

	send_to_userspace(&hdr_info, &rt_info, body_len, body);
	return RX_HANDLER_PASS;
}

void sniffer_clean(void) {
	if (real_dev) {
		rtnl_lock();
		netdev_rx_handler_unregister(real_dev);
		rtnl_unlock();
		dev_put(real_dev);
	}

	free_genl_family();

	if (mydev) {
		unregister_netdev(mydev);
		free_netdev(mydev);
	}
    printk(KERN_INFO "Sniffer unloaded\n");
}

int mywifi_open(struct net_device *dev) {
	printk(KERN_INFO "mywifi: device opened\n");
	return 0;
}
int mywifi_close(struct net_device *dev) {
	printk(KERN_INFO "mywifi: device closed\n");
	return 0;
}

netdev_tx_t mywifi_xmit(struct sk_buff *skb, struct net_device *dev) {
	dev_kfree_skb(skb);
	return NETDEV_TX_OK;
}

module_init(sniffer_init);
module_exit(sniffer_clean);


MODULE_AUTHOR("William Hansen-Baird");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A small project experimenting with WiFi packet sniffing :)");
MODULE_VERSION("1.0");
