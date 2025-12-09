#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include "../common/common.h"
#include "ctx.h"
#include "netlink/attr.h"

void handle_wifi_info(struct ctx *ctx, struct hdr_info *info, struct radiotap_info *rt_info, uint8_t *body, size_t body_len);
static int frame_handler(struct nl_msg *msg, void *arg);
#endif
