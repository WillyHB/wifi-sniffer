#include "sniffer_netlink.h"
#include <net/genetlink.h>
#include <linux/err.h>

static const struct genl_multicast_group sniffer_mcgrps[N_SNIFFER_MCGRPS] = {
	[SNIFFER_MCGRP_FRAMES] = { .name = SNIFFER_MCGRP_NAME }, // designated initializer (At index WIFI_MCGRP_FRAMES = { ... })
};

static struct genl_family sniffer_genl_family = {
	.name = SNIFFER_FAMILY_NAME,
	.version = 1,
	.maxattr = SNIFFER_ATTR_MAX,
	.mcgrps = sniffer_mcgrps,
	.n_mcgrps = N_SNIFFER_MCGRPS
};

void init_genl_family(void) {
	genl_register_family(&sniffer_genl_family);
}

void free_genl_family(void) {
	genl_unregister_family(&sniffer_genl_family);
}

void send_to_userspace(struct hdr_info *hdr_info, struct radiotap_info *rt_info, u8 body_len, u8 *body) {
	struct sk_buff *skb;
	size_t hdr_size = sizeof(struct hdr_info);
	size_t rt_size = sizeof(struct radiotap_info);
	void *msg_hdr;

	skb = genlmsg_new(NLMSG_GOODSIZE, GFP_ATOMIC);
	if (skb == NULL) {
		printk(KERN_ERR "Error Allocating Netlink Message Buffer");
		return;
	}

	msg_hdr = genlmsg_put(skb, 0, 0, &sniffer_genl_family, 0, SNIFFER_CMD_FRAME);
	if (msg_hdr == NULL) {
		printk(KERN_ERR "Error Creating Netlink Message Payload");
		kfree_skb(skb);
		return;
	}

	if (nla_put(skb, SNIFFER_ATTR_HEADER, hdr_size, hdr_info)
		|| nla_put(skb, SNIFFER_ATTR_RADIOTAP, rt_size, rt_info)
		|| nla_put(skb, SNIFFER_ATTR_BODY, body_len, body)) {
		printk(KERN_ERR "nla_put Failed\n");
		genlmsg_cancel(skb, msg_hdr);
		kfree_skb(skb);
		return;
	}

	genlmsg_end(skb, msg_hdr);

	int ret = genlmsg_multicast(&sniffer_genl_family, skb, 0, SNIFFER_MCGRP_FRAMES, GFP_ATOMIC);
	if (ret && ret != -3) {
		printk(KERN_ERR "Multicast failed (code %d)\n", ret);
	}
}
