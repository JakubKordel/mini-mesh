#include <stdlib.h>
#include <stdbool.h>
#include "InterestId.h"
#include "dataReceivedCallbackTable.h"
#include <meshSystemConfig.h>
#include <PacketsBuffer.h>
#include <packetStructs.h>
#include <packetsBuilder.h>
#include <neighborsTable.h>
#include <systemDependentHelpFunctions.h>
#include <meshSystemStruct.h>

int initDataReceivedCallbackTable(DataReceivedCallbackTable **drct) {
    if (drct == NULL) {
        return -1; 
    }
    
    *drct = (DataReceivedCallbackTable*)malloc(sizeof(DataReceivedCallbackTable));
    if (*drct == NULL) {
        return -1; 
    }
    
    (*drct)->mutex = binary_semaphore_create(1);
    (*drct)->firstUsed = MAX_DATA_CALLBACKS_NUM;  

    return 0;
}

int addDataCallback(DataReceivedCallbackTable *drct, InterestId interestId, void (*dataCallback)(void*, int), uint8_t maxDist) {
    if (drct == NULL || dataCallback == NULL) {
        return -1; 
    }

    binary_semaphore_wait(drct->mutex);
	
    int emptyNode = MAX_DATA_CALLBACKS_NUM;
    int i = drct->firstUsed;

	if (i > 0){
		emptyNode = 0;
	} 
	
	while (i != MAX_DATA_CALLBACKS_NUM) {
		if (compareInterestId(&drct->dataCallback[i].interestId, &interestId) == 0) {
			drct->dataCallback[emptyNode].interestId = interestId;
			drct->dataCallback[emptyNode].dataCallback = dataCallback;
			drct->dataCallback[emptyNode].maxDist = maxDist;
            binary_semaphore_post(drct->mutex);
            return 0;
		} 
		if (emptyNode == MAX_DATA_CALLBACKS_NUM && drct->dataCallback[i].next - i > 1){
			emptyNode = i+1;
		}
		i = drct->dataCallback[i].next;
	}


    if (emptyNode != MAX_DATA_CALLBACKS_NUM) {
		if (emptyNode != 0) {
			drct->dataCallback[emptyNode].next = drct->dataCallback[emptyNode-1].next;
			drct->dataCallback[emptyNode-1].next = emptyNode;
		} else {
			drct->dataCallback[emptyNode].next = drct->firstUsed;
			drct->firstUsed = 0;
		}
        drct->dataCallback[emptyNode].interestId = interestId;
        drct->dataCallback[emptyNode].dataCallback = dataCallback;
		drct->dataCallback[emptyNode].maxDist = maxDist;
        binary_semaphore_post(drct->mutex);
        return 0;
    }
	
    binary_semaphore_post(drct->mutex);
    return -1; 
}

int getDataCallback(DataReceivedCallbackTable *drct, InterestId interestId, void (**dataCallback)(void*, int)) {
    if (drct == NULL || dataCallback == NULL) {
        return -2; 
    }
    
    binary_semaphore_wait(drct->mutex);
    
    int i = drct->firstUsed;
    while (i < MAX_DATA_CALLBACKS_NUM) {
        if (compareInterestId(&drct->dataCallback[i].interestId, &interestId) == 0) {
            *dataCallback = drct->dataCallback[i].dataCallback;
            binary_semaphore_post(drct->mutex);
            return 0;
        }
        i = drct->dataCallback[i].next;
    }
    
    binary_semaphore_post(drct->mutex);
    return -1; 
}

int removeDataCallback(DataReceivedCallbackTable *drct, InterestId interestId) {
    if (drct == NULL) {
        return -1; 
    }
    
    binary_semaphore_wait(drct->mutex);
    
    int current = drct->firstUsed;
    int previous = MAX_DATA_CALLBACKS_NUM;

    while (current < MAX_DATA_CALLBACKS_NUM) {
        if (compareInterestId(&drct->dataCallback[current].interestId, &interestId) == 0) {
            if (previous < MAX_DATA_CALLBACKS_NUM) {
                drct->dataCallback[previous].next = drct->dataCallback[current].next;
            } else {
                drct->firstUsed = drct->dataCallback[current].next;
            }
            binary_semaphore_post(drct->mutex);
            return 0;
        }
        previous = current;
        current = drct->dataCallback[current].next;
    }
    
    binary_semaphore_post(drct->mutex);
    return -1; 
}

int broadcastSubscriptions(DataReceivedCallbackTable *drct, MeshSystem * ms, uint32_t timestamp){
	if (drct == NULL) {
        return -1; 
    }
    
    binary_semaphore_wait(drct->mutex);

	int i = drct->firstUsed;
	while (i < MAX_DATA_CALLBACKS_NUM) {
        
		void * packet = NULL; 
		uint16_t packetLen;
	
		uint32_t uniqueSendersNumber = rand() % UINT32_MAX;
		
		uint32_t connectedMask;
	
		PacketHeader header;
		header.meshId = ms->meshId;
		header.sourceId = ms->nodeId;
		gradientHandleInterestArrival(ms->gradientTable, drct->dataCallback[i].interestId, uniqueSendersNumber, 0U, timestamp, &header.destinationId);
		getConnectedMask(ms->neighborsTable, &connectedMask);
		
		//topic start
		//until interest path reinforncments are not properly implemented interest packets have to be flooded everywhere with same rate so two data sources case is handled properly:
		//header.destinationId &= connectedMask;
		header.destinationId = connectedMask; 
		//topic end

		header.totalLength = PACKET_HEADER_SIZE + INTEREST_FLOOD_PACKET_HEADER_SIZE;
		header.packetDescr = (uint8_t)(DESCR_INTEREST_FLOOD_PACKET_BITS | DESCR_IS_MULTICAST_BIT );
		if (REQ_ACK_ON_INTEREST_PCKTS == 1) {
			header.packetDescr |= DESCR_REQ_ACK_BIT;
		}
		header.packetId = 0U;
	
		InterestFloodPacket rest;
		rest.interestId = drct->dataCallback[i].interestId;
		rest.uniqueSendersNumber = uniqueSendersNumber;
		rest.hopsCount = 0U;
		rest.hopsLimit = drct->dataCallback[i].maxDist;
	
		buildInterestFloodPacket(&packet, &packetLen, &header, &rest);
	
		if (header.destinationId != 0U){
			putPacketToBuffer(ms->packetsBuffer, header.destinationId, packet, packetLen);
		}
		destroyPacket(&packet);
		
        i = drct->dataCallback[i].next;
    }
	
	binary_semaphore_post(drct->mutex);
    return 0;
}

int freeDataCallbackTable(DataReceivedCallbackTable *drct) {
    if (drct == NULL) {
        return -1; 
    }
    
    binary_semaphore_destroy(drct->mutex);
    
    free(drct);
    return 0;
}
