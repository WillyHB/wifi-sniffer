#include "action_frame_parser.h"
#include <stdint.h>

const char *action_to_string(ACTION_CAT cat, uint8_t code) {
	switch (cat) {
		case SPECTRUM_MANAGEMENT: return spectrummgmt_action_to_string(code);
		case QOS: return qos_action_to_string(code);
		case DLS: return dls_action_to_string(code);
		case BLOCK_ACK: return blockack_action_to_string(code);
		case PUBLIC: return "PUBLIC";
		case RADIO_MEASUREMENT: return rmeasurement_action_to_string(code);
		case FAST_BSS_TRANSITION: return fbsstransition_action_to_string(code);
		case HT: return ht_action_to_string(code);
		case SA_QUERY: return saquery_action_to_string(code);
		case PROTECTED_DUAL_OF_PUBLIC: return "PROTECTED DUAL OF PUBLIC";
		case VHT: return "VERY HIGH THROUGHPUT";
		case HE: return "HE";
	}

	return "UNKNOWN";
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

const char *spectrummgmt_action_to_string(SPECTRUM_MANAGEMENT_ACTION code) {
	switch (code) {
		case MEASUREMENT_REQUEST: return "MEASUREMENT REQUEST";
		case MEASUREMENT_REPORT: return "MEASUREMENT REPORT";
		case TPC_REQUEST: return "TPC REQUEST";
		case TPC_REPORT: return "TPC REPORT";
		case CHANNEL_SWITCH_ANNOUNCEMENT: return "CHANNEL SWITCH ANNOUNCEMENT";
	}

	return "UNKNOWN";
}

const char *qos_action_to_string(QOS_ACTION code) {
	switch (code) {
		case ADDTS_REQUEST: return "ADDTS REQUEST";
		case ADDTS_RESPONSE: return "ADDTS RESPONSE";
		case DELTS: return "DELTS";
		case SCHEDULE: return "SCHEDULE";
		case QOS_MAP_CONF: return "QoS MAP CONFIG";
	}

	return "UNKNOWN";
}

const char *dls_action_to_string(DLS_ACTION code){
	switch (code) {
		case DLS_REQUEST: return "DLS REQUEST";
		case DLS_RESPONSE: return "DLS RESPONSE";
		case DLS_TEARDOWN: return "DLS TEARDOWN";
	}

	return "UNKNOWN";
}


const char *blockack_action_to_string(BLOCK_ACK_ACTION code){
	switch (code) {
		case ADDBA_REQUEST: return "ADDBA REQUEST";
		case ADDBA_RESPONSE: return "ADDBA RESPONSE";
		case DELBA: return "DELBA";
	}

	return "UNKNOWN";
}


const char *rmeasurement_action_to_string(RADIO_MEASUREMENT_ACTION code){
	switch (code) {
		case RADIO_MEASUREMENT_REQUEST: return "RADIO MEASUREMENT REQUEST";
		case RADIO_MEASUREMENT_REPORT: return "RADIO MEASUREMENT REPORT";
		case LINK_MEASUREMENT_REQUEST: return "LINK MEASUREMENT REQUEST";
		case LINK_MEASUREMENT_REPORT: return "LINK MEASUREMENT REPORT";
		case NEIGHBOUR_REPORT_REQUEST: return "NEIGHBOUR REPORT REQUEST";
		case NEIGHBOUR_REPORT_RESPONSE: return "NEIGHBOUR REPORT RESPONSE";
	}

	return "UNKNOWN";
}


const char *fbsstransition_action_to_string(FAST_BSS_TRANSITION_ACTION code){
	switch (code) {
		case FT_REQUEST: return "FT REQUEST";
		case FT_RESPONSE: return "FT RESPONSE";
		case FT_CONFIRM: return "FT CONFIRM";
		case FT_ACK: return "FT ACK";
	}

	return "UNKNOWN";
}


const char *ht_action_to_string(HT_ACTION code){
	switch (code) {
		case NOTIFY_CHANNEL_WIDTH: return "NOTIFY CHANNEL WIDTH";
		case SM_POWER_SAVE: return "SM POWER SAVE";
		case PMSP: return "PMSP";
		case SET_PCO_PHASE: return "SET PCO PHASE";
		case CSI: return "CSI";
		case NONCOMPRESSED_BEAMFORMING: return "NON-COMPRESSED BEAMFORMING";
		case COMPRESSED_BEAMFORMING: return "COMPRESSED BEAMFORMING";
		case ASEL_INDICES_FEEDBACK: return "ASEL INDICES FEEDBACK";
	}

	return "UNKNOWN";
}


const char *saquery_action_to_string(SA_QUERY_ACTION code){
	switch (code) {
		case SA_QUERY_REQUEST: return "SA QUERY REQUEST";
		case SA_QUERY_RESPONSE: return "SA QUERY RESPONSE";
	}

	return "UNKNOWN";
}


