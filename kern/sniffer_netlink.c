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

void send_to_userspace(struct wifi_frame_info *info) {
	struct sk_buff *skb;
	size_t info_size = sizeof(struct wifi_frame_info);
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

	if (nla_put(skb, WIFI_ATTR_FRAME, info_size, info)) {
		printk(KERN_ERR "Error with nla_put\n");
		genlmsg_cancel(skb, msg_hdr);
		kfree_skb(skb);
	}

	genlmsg_end(skb, msg_hdr);

	int ret = genlmsg_multicast(&wifi_genl_family, skb, 0, WIFI_MCGRP_FRAMES, GFP_ATOMIC);
	if (ret) {
		printk(KERN_ERR "Multicast failed (code %d)\n", ret);
	}
}
