#ifndef SNIFFER_NETLINK_H
#define SNIFFER_NETLINK_H

#include "../common/common.h"
#include <net/genetlink.h>


static const struct genl_multicast_group wifi_mcgrps[N_WIFI_MCGRPS] = {
	[WIFI_MCGRP_FRAMES] = { .name = "wifi_frames" }, // designated initializer (At index WIFI_MCGRP_FRAMES = { ... })
};

static struct genl_family wifi_genl_family = {
	.name = WIFI_FAMILY_NAME,
	.version = 1,
	.maxattr = WIFI_ATTR_MAX,
	.mcgrps = wifi_mcgrps,
	.n_mcgrps = N_WIFI_MCGRPS,
};

#endif

