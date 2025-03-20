#ifndef MESHSYSTEMCONFIG_H_
#define MESHSYSTEMCONFIG_H_

#define 	MAX_NEIGHBORS_NUM					32  // FIXED, it might be possible to change it to lower values (doesn't make much sense) but right now 32 is upper limit (uint32_t as mask for direct neighbors in system limits this number to 32, same with field related to thisin packets), maximal amount of direct neighbors
#define		PCKTS_BUFF_SPACE_BYTES				10000 // CUSTOMIZABLE, to reduce used memory use lower values, if buffer is ofhen overloaded then its probs better to use lower data rate
#define		MAX_PCKTS_IN_BUFFER_NUM				100 // CUSTOMIZABLE, to reduce used memory use lower values, if buffer is ofhen overloaded then its probs better to use lower data rate
#define		NEIGHBOR_INACTIVITY_TIMEOUT_SEC		15 // CUSTOMIZABLE, 		
#define		GRADIENT_INACTIVITY_TIMEOUT_SEC		60 // CUSTOMIZABLE
#define 	ACK_TIMEOUT_DS						2 // CUSTOMIZABLE
#define		MAX_RETRANSMISSIONS_NUM				3 // CUSTOMIZABLE
#define		REQ_ACK_ON_DATA_PCKTS				1 // CUSTOMIZABLE
#define		REQ_ACK_ON_INTEREST_PCKTS			1 // CUSTOMIZABLE
#define		MAX_TRANSMIT_PCKT_SIZE_BYTES		1100 // ENVIRONMENT DEPENDENT, for esp8266 using esp now library to transmit packets maximum is unfortunately 240 for library packets
#define		WIFI_CHANNEL						4	//CUSTOMIZABLE, matters when wifi is used as packets transmission technology 
#define		TRANSMISSIONS_BASE_DELAY_MS			1 //CUSTOMIZABLE
#define		MAX_GRADIENTS_NUM					50 //CUSTOMIZABLE, maximal number of stored gradients in gradients table
#define		MAX_INTEREST_CALLBACKS_NUM			10	//CUSTOMIZABLE, maximal number of stored interest received callback functions
#define		MAX_DATA_CALLBACKS_NUM				40	//CUSTOMIZABLE, maximal number of stored data received callback functions 
#define		ENCRYPT_PACKETS						1	//CUSTOMIZABLE, should packets be encrypted, 1 YES, 0 NO
#define		SYSTEM_MANAGEMENT_PROCESS_PRIORITY	10	// CUSTOMIZABLE & ENVIRONMENT DEPENDENT
#define		TRANSMIT_DATA_PROCESS_PRIORITY		10	// CUSTOMIZABLE & ENVIRONMENT DEPENDENT
#define		INPUT_PCKT_HNDL_PROCESS_PRIORITY	11	// CUSTOMIZABLE & ENVIRONMENT DEPENDENT
			
#endif /* MESHSYSTEMCONFIG_H_ */
