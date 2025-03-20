#include <InterestId.h>
#include <meshAPI.h>
#include "../simulServer/simulServer.h"

#include <stdlib.h> 
#include <time.h>
#include <unistd.h>    
#include <string.h>

#define MESH_ID 1

#define MIRROR_ID_PART_1 0x00000000
#define MIRROR_ID_PART_2 0x00000000
#define MIRROR_ID_PART_3 0x00000000
#define MIRROR_ID_PART_4 0x00000002

#define MIRRORED_DATA_ID_PART_1 0x00000000
#define MIRRORED_DATA_ID_PART_2 0x00000000
#define MIRRORED_DATA_ID_PART_3 0x00000000
#define MIRRORED_DATA_ID_PART_4 0x00000003

#define MAX_DIST 8

InterestId mirroredDataId;
mesh_network_handle_t *mnh = NULL;
	
void dataToMirrorReceived(void *data, int size) {

    char *mirrored_data = malloc(size);
    if (mirrored_data == NULL) {
        return;
    }

    for (int i = 0; i < size; i++) {
        mirrored_data[i] = *((char*)data + size - i - 1);
    }
	
    sendDataAlongGradient(mnh, &mirroredDataId, mirrored_data, size);

    free(mirrored_data);
}

void mirrorNode(){

	InterestId mirrorId;

    	makeInterestId(&mirrorId, MIRROR_ID_PART_1, MIRROR_ID_PART_2, MIRROR_ID_PART_3, MIRROR_ID_PART_4);
    	makeInterestId(&mirroredDataId, MIRRORED_DATA_ID_PART_1, MIRRORED_DATA_ID_PART_2, MIRRORED_DATA_ID_PART_3, MIRRORED_DATA_ID_PART_4);
    
	uint8_t encrKey[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

  	joinNetwork(&mnh, MESH_ID, encrKey);
  	
  	subscribe(mnh, &mirrorId, MAX_DIST, dataToMirrorReceived);
  	publish(mnh, &mirroredDataId, NULL, (void *) mnh);
  	
  	while(1){
  		sleep(100);
  	}
}

