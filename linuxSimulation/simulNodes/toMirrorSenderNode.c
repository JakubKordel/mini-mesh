#include <InterestId.h>
#include <meshAPI.h>
#include <unistd.h> 
#include "../simulServer/simulServer.h"

#include <stdlib.h> 
#include <time.h>   

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

#define MSG_SIZE 23
#define MSGS_NUM 100


bool firstReceived = false;
struct timespec firstMsgTimestamp, lastMsgTimestamp;
int receivedBytesCounter = 0;

void receivedMirroredDataHandler(void *data, int size){
    linuxSimulationPrint("Received mirrored data: %.*s (Size: %d)\n", size, (char*)data + sizeof(int), size - sizeof(int));
    
    int msgIndex;
    
    memcpy(&msgIndex, data, sizeof(int));
    linuxSimulationPrint("Msg Index: %d \n", msgIndex);
    
    clock_gettime(CLOCK_MONOTONIC, &lastMsgTimestamp);
    
    if (!firstReceived){
    	firstMsgTimestamp = lastMsgTimestamp;
    	firstReceived = true;
    }
    
    receivedBytesCounter += size;
}


void fillMsg(char *msg, int msgSize, char *pattern, int patternSize) {
    int patternIndex = 0;
    
    for (int i = 0; i < msgSize; i++) {
        msg[i] = pattern[patternIndex];
        patternIndex = (patternIndex + 1) % patternSize;
    }
}

void toMirrorSenderNode(){

    InterestId mirrorId;
    InterestId mirroredDataId;

    makeInterestId(&mirrorId, MIRROR_ID_PART_1, MIRROR_ID_PART_2, MIRROR_ID_PART_3, MIRROR_ID_PART_4);
    makeInterestId(&mirroredDataId, MIRRORED_DATA_ID_PART_1, MIRRORED_DATA_ID_PART_2, MIRRORED_DATA_ID_PART_3, MIRRORED_DATA_ID_PART_4);
    
    uint8_t encrKey[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

    mesh_network_handle_t *mnh = NULL;
    joinNetwork(&mnh, MESH_ID, encrKey);
  	
    subscribe(mnh, &mirroredDataId, MAX_DIST, receivedMirroredDataHandler);
    publish(mnh, &mirrorId, NULL, (void *) mnh);

    char tenBytesMsg[11] = "gsMsetyB01";
    char msg[MSG_SIZE];
    
    fillMsg(msg, MSG_SIZE, tenBytesMsg, 10);
    
    sleep(10);
    
    for (int i = 0; i < MSGS_NUM; i++){ 
    	//mirror msg index (bytes of int i) and place on the end of msg
        for (int j = 0; j < sizeof(int); j++) {
    		msg[MSG_SIZE - sizeof(int) + j] = *(((char*)(&i)) + sizeof(int) - 1 - j) ;
	}
        
        sendDataAlongGradient(mnh, &mirrorId, msg, MSG_SIZE);
        linuxSimulationPrint("Sending data: %s (Size: %d)\n", msg, MSG_SIZE);
        sleep(1);
    }
    
    while (1){
     	long elapsed_ms = (lastMsgTimestamp.tv_sec - firstMsgTimestamp.tv_sec) * 1000 +
                      (lastMsgTimestamp.tv_nsec - firstMsgTimestamp.tv_nsec) / 1000000;

    	linuxSimulationPrint("Elapsed time: %ld milliseconds\n", elapsed_ms);
    	linuxSimulationPrint("Received Bytes: %d \n", receivedBytesCounter);
    	
    	sleep(5);
    }
}
