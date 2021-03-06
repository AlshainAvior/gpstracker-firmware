/**
 * Configure how the positions are backuped with the network
 */

#pragma once

#include "User.h"

#define POSITIONS_CONFIG_NET_SAVE_THRESHOLD		30									///< Determines how many positions will be saved before a network backup is needed (only when not moving though).
#define POSITIONS_CONFIG_NET_APN				"Vodafone"							///< APN used for GPRS context
#define POSITIONS_CONFIG_NET_EXPECTED_RESPONSE	201									///< Expected response code from the server that indicates that the positions has been successfully backuped.
#define POSITIONS_CONFIG_NET_UNAVAILABLE_NETWORK_POSTPONE_THRESHOLD		5			///< Determines how many times to deal with an unreliable network before postponing the backup.
																					///< In an area where cell reception isn't good, this avoid to try to backup the positions
																					///< every single time, which would rapidly consumes all the battery.

struct networkConfig_t {
	uint8_t saveThreshold;		//sizeof = 1
	uint16_t lastSavedEntry;	//sizeof = 2
	char apn[20];				//sizeof = 20
	char url[50];				//sizeof = 50
};								//sizeof = 73
