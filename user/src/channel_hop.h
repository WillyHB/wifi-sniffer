#ifndef CHANNEL_HOP_H
#define CHANNEL_HOP_H

#include <linux/nl80211.h>
#include <net/if.h>

#define CHANNELS_COUNT 7 // Number of channels we hop

//void channel_hop(const char *interface);
int channel_init(const char *interface);
int channel_to_freq(int channel);
int set_channel(const char *interface, int channel);
int next_channel(const char *interface);
int current_channel();
#endif
