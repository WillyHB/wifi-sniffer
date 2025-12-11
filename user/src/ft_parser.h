#ifndef FT_PARSER_H
#define FT_PARSER_H

#include <stdint.h>
#include "../../common/common.h"

typedef enum FRAME_TYPE {
	T_MANAGEMENT,
	T_CONTROL,
	T_DATA
} FRAME_TYPE;

typedef enum MANAGEMENT_TYPE {
	M_ASSOCIATION_REQUEST	= 0b0000,
	M_ASSOCIATION_RESPONSE	= 0b0001,
	M_REASSOCIATION_REQUEST	= 0b0010,
	M_REASSOCIATION_RESPONSE= 0b0011,
	M_PROBE_REQUEST			= 0b0100,
	M_PROBE_RESPONSE		= 0b0101,
	M_TIMING_ADVERTISEMENT	= 0b0110,
	M_RESERVED1				= 0b0111,
	M_BEACON				= 0b1000,
	M_ATIM					= 0b1001,
	M_DISASSOCIATION		= 0b1010,
	M_AUTHENTICATION		= 0b1011,
	M_DEAUTHENTICATION		= 0b1100,
	M_ACTION				= 0b1101,
	M_ACTION_NOACK			= 0b1110,
	M_RESERVED2				= 0b1111,
} MANAGEMENT_TYPE;

typedef enum CONTROL_TYPE {
	C_RESERVED1					= 0b0000,
	C_RESERVED2					= 0b0001,
	C_TRIGGER					= 0b0010,
	C_TACK						= 0b0011,
	C_BEAMFORMING_REPORT_POLL	= 0b0100,
	C_NDP_ANNOUNCEMENT			= 0b0101,
	C_CONTROL_FRAME_EXT			= 0b0110,
	C_CONTROL_WRAPPER			= 0b0111,
	C_BLOCK_ACK_REQUEST			= 0b1000,
	C_BLOCK_ACK					= 0b1001,
	C_PS_POLL					= 0b1010,
	C_RTS						= 0b1011,
	C_CTS						= 0b1100,
	C_ACK						= 0b1101,
	CF_END						= 0b1110,
	CF_END_CF_ACK				= 0b1111,
} CONTROL_TYPE;

typedef enum DATA_TYPE {
	D_DATA		= 0b0000,
	D_NULL		= 0b0100,
	D_QOS_DATA = 0b1000,
	D_QOS_NULL = 0b1100,
	D_RESERVED = 0b1101,
} DATA_TYPE;


struct frame_type {
	FRAME_TYPE ft;
	union {
		MANAGEMENT_TYPE mst;
		CONTROL_TYPE cst;
		DATA_TYPE dst;
	};
};



const char *frame_direction_to_string(FRAME_DIRECTION dir);
const char *sub_frame_string(FRAME_TYPE t, uint8_t st);
const char *frame_t_to_string(FRAME_TYPE t);
const char *m_frame_to_string(MANAGEMENT_TYPE t);
const char *c_frame_to_string(CONTROL_TYPE t);
const char *d_frame_to_string(DATA_TYPE t);
uint8_t mgmt_fixed_params(MANAGEMENT_TYPE frame_st);
#endif
