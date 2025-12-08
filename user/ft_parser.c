#include "ft_parser.h"

const char *frame_t_to_string(FRAME_TYPE t) {
	switch (t) {
		case T_MANAGEMENT: return "MANAGEMENT";
		case T_CONTROL: return "CONTROL";
		case T_DATA: return "DATA";
	}
	return "UNKNOWN";
}

const char *m_frame_to_string(MANAGEMENT_TYPE t) {
	switch (t) {
		case M_ASSOCIATION_REQUEST: return "ASSOCIATION REQUEST";
		case M_REASSOCIATION_REQUEST: return "REASSOCIATION REQUEST";
		case M_PROBE_REQUEST: return "PROBE REQUEST";
		case M_ASSOCIATION_RESPONSE: return "ASSOCIATION RESPONSE";
		case M_REASSOCIATION_RESPONSE: return "REASSOCIATION RESPONSE";
		case M_PROBE_RESPONSE: return "PROBE RESPONSE";
		case M_TIMING_ADVERTISEMENT: return "TIMING ADVERTISEMENT";
		case M_RESERVED: return "RESERVED";
		case M_ACTION_NOACK: return "NO ACK";
		case M_BEACON: return "BEACON";
		case M_DISASSOCIATION: return "DISASSOCIATION";
		case M_AUTHENTICATION: return "AUTHENTICATION";
		case M_DEAUTHENTICATION: return "DEAUTHENTICATION";
		case M_ACTION: return "ACTION";						
	}

	return "UNKNOWN";
}
const char *c_frame_to_string(CONTROL_TYPE t) {
	switch (t) {
		case C_BEAMFORMING_REPORT_POLL: return "BEAMFORMING REPORT POLL";
		case C_NDP_ANNOUNCEMENT: return "NDP ANNOUNCEMENT";
		case C_CONTROL_FRAME_EXT: return "CONTROL FRAME EXTENSION";
		case C_CONTROL_WRAPPER: return "CONTROL WRAPPER";
		case C_BLOCK_ACK_REQUEST: return "BLOCK ACK REQUEST";
		case C_BLOCK_ACK: return "BLOCK ACK";
		case C_PS_POLL: return "PS POLL";
		case C_RTS: return "RTS";
		case C_CTS: return "CTS";
		case C_ACK: return "ACK";
	}
	return "UNKNOWN";
}

const char *d_frame_to_string(DATA_TYPE t) {
	switch (t) {
		case D_DATA: return "DATA";
		case D_NULL: return "NULL DATA";
		case D_QOS_DATA: return "QoS DATA";
		case D_QOS_NULL: return "QoS NULL DATA";
		case D_RESERVED: return "RESERVED DATA";
	}
	return "UNKNOWN";
}

const char *sub_frame_string(FRAME_TYPE t, uint8_t st) {
	switch (t) {
		case T_MANAGEMENT: return m_frame_to_string(st);
		case T_CONTROL: return c_frame_to_string(st);
		case T_DATA: return d_frame_to_string(st);
	}
	return "UNKNOWN";
}

const char *frame_direction_to_string(FRAME_DIRECTION dir) {
	switch (dir) {
		case CLIENT_TO_CLIENT: return "CLIENT TO CLIENT";
		case CLIENT_TO_AP: return "CLIENT TO AP";
		case AP_TO_CLIENT: return "AP TO CLIENT";
		case AP_TO_AP: return "AP TO AP";
	}
	return "UNKNOWN";
}

uint8_t mgmt_fixed_params(MANAGEMENT_TYPE frame_st) {
	switch (frame_st) {
		case M_BEACON:
		case M_TIMING_ADVERTISEMENT:
		case M_PROBE_RESPONSE: return 12;
		case M_RESERVED:
		case M_ACTION:
		case M_ACTION_NOACK:
		case M_DISASSOCIATION:
		case M_DEAUTHENTICATION: return 2;
		case M_PROBE_REQUEST: return 0;
		case M_ASSOCIATION_REQUEST: return 4;
		case M_AUTHENTICATION: 
		case M_ASSOCIATION_RESPONSE:
		case M_REASSOCIATION_REQUEST:
		case M_REASSOCIATION_RESPONSE: return 6;
	}
	return 0;
}

const char *action_cat_to_string(ACTION_CAT cat) {
	switch (cat) {
		case SPECTRUM_MANAGEMENT: return "SPECTRUM MANAGEMENT";
		case QOS: return "QoS";
		case DLS: return "DLS";
		case BLOCK_ACK: return "BLOCK ACK";
		case PUBLIC: return "PUBLIC";
		case RADIO_MEASUREMENT: return "RADIO MEASUREMENT";
		case FAST_BSS_TRANSITION: return "FAST BSS TRANSITION";
		case HT: return "HIGH THROUGHPUT";
		case SA_QUERY: return "SA QUERY";
		case PROTECTED_DUAL_OF_PUBLIC: return "PROTECTED DUAL OF PUBLIC";
		case VHT: return "VERY HIGH THROUGHPUT";
		case HE: return "HE";
	}

	return "UNKNOWN";
}
