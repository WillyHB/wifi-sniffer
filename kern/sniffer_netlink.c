#include "sniffer_netlink.h"
#include <net/genetlink.h>
#include <linux/err.h>

static const struct genl_multicast_group wifi_mcgrps[N_WIFI_MCGRPS] = {
	[WIFI_MCGRP_FRAMES] = { .name = WIFI_MCGRP_NAME }, // designated initializer (At index WIFI_MCGRP_FRAMES = { ... })
};

static struct genl_family wifi_genl_family = {
	.name = WIFI_FAMILY_NAME,
	.version = 1,
	.maxattr = WIFI_ATTR_MAX,
	.mcgrps = wifi_mcgrps,
	.n_mcgrps = N_WIFI_MCGRPS
};

void init_genl_family(void) {
	genl_register_family(&wifi_genl_family);
}

void free_genl_family(void) {
	genl_unregister_family(&wifi_genl_family);
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

	msg_hdr = genlmsg_put(skb, 0, 0, &wifi_genl_family, 0, WIFI_SNIF_CMD_FRAME);
	if (msg_hdr == NULL) {
		printk(KERN_ERR "Error Creating Netlink Message Payload");
		kfree_skb(skb);
		return;
	}

	if (nla_put(skb, WIFI_ATTR_HEADER, hdr_size, hdr_info)
		|| nla_put(skb, WIFI_ATTR_RADIOTAP, rt_size, rt_info)
		|| nla_put(skb, WIFI_ATTR_BODY, body_len, body)) {
		printk(KERN_ERR "nla_put Failed\n");
		genlmsg_cancel(skb, msg_hdr);
		kfree_skb(skb);
		return;
	}

	genlmsg_end(skb, msg_hdr);

	int ret = genlmsg_multicast(&wifi_genl_family, skb, 0, WIFI_MCGRP_FRAMES, GFP_ATOMIC);
	if (ret && ret != -3) {
		printk(KERN_ERR "Multicast failed (code %d)\n", ret);
	}

	//kfree_skb(skb);
}
