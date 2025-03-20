#include <InterestId.h>
#include <meshAPI.h>
#include <unistd.h> 

#define MESH_ID 1

#define INTEREST_ID_PART_1 0x00000000
#define INTEREST_ID_PART_2 0x00000000
#define INTEREST_ID_PART_3 0x00000000
#define INTEREST_ID_PART_4 0x00000001

#define MAX_DIST 10

void relayingNode(){

	InterestId interestId;
    	makeInterestId(&interestId, INTEREST_ID_PART_1, INTEREST_ID_PART_2, INTEREST_ID_PART_3, INTEREST_ID_PART_4);
    
	uint8_t encrKey[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

  	mesh_network_handle_t *mnh = NULL;
  	joinNetwork(&mnh, MESH_ID, encrKey);
  	while(1){
  		sleep(100);
  	}
	//leaveNetwork(&mnh);

}
