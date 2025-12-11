#include "channel_hop.h"
#include "netlink/genl/ctrl.h"
#include "netlink/genl/genl.h"
#include "netlink/msg.h"
#include "netlink/netlink.h"
#include "netlink/socket.h"

int channels[CHANNELS_COUNT] = {1,6,11, 36, 40, 44, 48};
static int channel = 0;

int channel_init(const char *interface) {
	return set_channel(interface, channels[channel]);
}

int channel_to_freq(int channel) {
    if (channel >= 1 && channel <= 13) return 2407 + channel * 5;
    if (channel == 14) return 2484;
    if (channel >= 36 && channel <= 177) return 5000 + channel * 5;
    return -1;
}

int current_channel() {
	return channels[channel];
}

int next_channel(const char *interface) {
	int ret =  set_channel(interface, channels[channel]);
	channel = (channel + 1) % CHANNELS_COUNT;
	return ret;
}

int set_channel(const char *interface, int channel) {
	struct nl_sock *sock = nl_socket_alloc();
	if (sock == NULL) return -1;
	if (genl_connect(sock)) {
		return -1;
	}

	int id = genl_ctrl_resolve(sock, "nl80211");

	struct nl_msg *msg = nlmsg_alloc();
	if (msg == NULL) {
		nl_socket_free(sock);
		return -1;
	}

	int freq = channel_to_freq(channel);
	int if_index = if_nametoindex(interface);

	//nl_msg, port, seq, family hdrlen, flags, cmd, version
	genlmsg_put(msg, 0, 0, id, 0, 0, NL80211_CMD_SET_WIPHY, 0);
    nla_put_u32(msg, NL80211_ATTR_WIPHY_FREQ, freq);
	nla_put_u32(msg, NL80211_ATTR_IFINDEX, if_index);

	if (nl_send_auto_complete(sock, msg) < 0) {
		nlmsg_free(msg);
		nl_socket_free(sock);
		return -1;
	}

	int ret =  nl_wait_for_ack(sock);
	nlmsg_free(msg);
	nl_socket_free(sock);
	return ret;
}
