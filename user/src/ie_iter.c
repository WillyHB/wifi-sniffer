#include "ie_iter.h"
#include "main.h"
#include <stdlib.h>

int is_printable(uint8_t *str, uint64_t len) {
	for (uint64_t i = 0; i < len; i++) {
		if (str[i] < 32 || str[i] > 126) {
			return 0;
		}
	}

	return 1;
}

void iter_packet_ies(struct ap *ap, struct packet_info *info, uint8_t *body, size_t body_len) {
	struct ie_iterator *iter = ie_iter(body, body_len, info->hdr.frame_st);

	while (!ie_iter_next(iter)) {
		if (iter->cur_id == 0) {
			if (memcmp(ap->ssid, iter->cur, iter->cur_len) != 0 && is_printable(iter->cur, iter->cur_len)) {
				memcpy(ap->ssid, iter->cur, iter->cur_len);
				ap->ssid[iter->cur_len] = 0;
				ap->ssid_len = iter->cur_len;
			}
		}
	}
	ie_iter_free(iter);
}
struct ie_iterator *ie_iter(uint8_t *body, uint8_t body_len, MANAGEMENT_TYPE frame_st) {

	uint8_t fixed = mgmt_fixed_params(frame_st);

	uint8_t *ptr = body + fixed;
	int ptr_len = body_len - fixed;
	if (ptr_len <= 0) return NULL;

	struct ie_iterator *iter = malloc(sizeof(struct ie_iterator) + ptr_len); //ptr_len for variable array
	iter->ies_len = ptr_len;
	memcpy(iter->ies, ptr, iter->ies_len);

	iter->cur = NULL;
	iter->cur_id = 0;
	iter->cur_len = 0;
	return iter;
}

int ie_iter_next(struct ie_iterator *iter) {
	if (iter == NULL) return 1;

	if (iter->cur == NULL) {
		if (iter->ies_len <= 2) {
			return 1;
		}

		iter->cur = iter->ies+2;
		iter->cur_id = iter->ies[0];
		iter->cur_len = iter->ies[1];
	} else {
		iter->cur += iter->cur_len;
		if (iter->cur - &iter->ies[0] >= iter->ies_len) {
			return 1;
		}

		iter->cur_id = iter->cur[0];
		iter->cur_len = iter->cur[1];
		iter->cur += 2;
		return 0;
	}

	return 0;
}

void ie_iter_free(struct ie_iterator *iter) {
	if (iter == NULL) return;
	free(iter);
}
