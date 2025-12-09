#ifndef IE_ITER_H
#define IE_ITER_H

#include "ft_parser.h"
#include "main.h"
#include <stdint.h>
struct ie_iterator {
	uint8_t cur_id;
	uint8_t cur_len;
	uint8_t *cur;

	uint8_t ies_len;
	uint8_t ies[];
};

struct ie_iterator *ie_iter(uint8_t *body, uint8_t body_len, MANAGEMENT_TYPE frame_st);
int ie_iter_next(struct ie_iterator*);
void ie_iter_free(struct ie_iterator*);
void iter_packet_ies(struct ap *ap, struct packet_info *info, uint8_t *body, size_t body_len);
#endif
