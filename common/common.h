#ifndef COMMON_H
#define COMMON_H

#include "asm-generic/int-ll64.h"

enum {
	WIFI_ATTR_UNSPEC,
	WIFI_ATTR_RADIOTAP,
	WIFI_ATTR_HEADER,
	WIFI_ATTR_BODY,
	__WIFI_ATTR_MAX,
};

#define WIFI_FAMILY_NAME "wifi_sniffer"
#define WIFI_MCGRP_NAME "wifi_frame"

#define WIFI_SNIF_CMD_FRAME 1
#define WIFI_MCGRP_FRAMES 0
#define WIFI_ATTR_MAX (__WIFI_ATTR_MAX - 1)
#define N_WIFI_MCGRPS 1

typedef enum FRAME_DIRECTION {
	CLIENT_TO_CLIENT,
	CLIENT_TO_AP,
	AP_TO_CLIENT,
	AP_TO_AP,
} FRAME_DIRECTION;

#endif
