#include <stdlib.h>
#include "neighborsTable.h"
#include <meshLibraryProcesses.h>

#include <meshSystemConfig.h>

#include <stdio.h>

int initNeighborTable(NeighborsTable **nt) {
    *nt = (NeighborsTable *)malloc(sizeof(NeighborsTable));
    memset(*nt, 0, sizeof(NeighborsTable));
    (*nt)->connectedMask = 0U;

    for (int i = 0; i < MAX_NEIGHBORS_NUM; i++) {
        (*nt)->neighbor[i].selfMulticastBitNumber = MAX_NEIGHBORS_NUM;
        (*nt)->neighbor[i].connectionState = DISCONNECTED;
    }

    (*nt)->firstNonDisconnected = MAX_NEIGHBORS_NUM;

    (*nt)->mutex = binary_semaphore_create(1);
    return 0;
}

int neighborHeardHandle(NeighborsTable *nt, uint32_t name, uint32_t connectionTimestamp) {
    binary_semaphore_wait(nt->mutex);

    int emptyNode = MAX_NEIGHBORS_NUM;
    int i = nt->firstNonDisconnected;

	if (i > 0){
		emptyNode = 0;
	} 
	
	while (i != MAX_NEIGHBORS_NUM) {
		if (nt->neighbor[i].name == name) {
			nt->neighbor[i].lastContactTimestamp = connectionTimestamp;
			binary_semaphore_post(nt->mutex);
			return 0; //updated neighbor
		} 
		if (emptyNode == MAX_NEIGHBORS_NUM && nt->neighbor[i].next - i > 1){
			emptyNode = i+1;
		}
		i = nt->neighbor[i].next;
	}
		
    if (emptyNode != MAX_NEIGHBORS_NUM) {
		if (emptyNode != 0) {
			nt->neighbor[emptyNode].next = nt->neighbor[emptyNode-1].next;
			nt->neighbor[emptyNode-1].next = emptyNode;
		} else {
			nt->neighbor[emptyNode].next = nt->firstNonDisconnected;
			nt->firstNonDisconnected = 0;
		}
        nt->neighbor[emptyNode].connectionState = NO_MULTICAST_BIT;
        nt->neighbor[emptyNode].name = name;
        nt->neighbor[emptyNode].lastContactTimestamp = connectionTimestamp;
        nt->connectedMask |= (1U << emptyNode);

        binary_semaphore_post(nt->mutex);
        return 1; //added new neighor
    }

    binary_semaphore_post(nt->mutex);
    return -1; //no empty space
}

int disconnectSilentNeighbors(NeighborsTable *nt, uint32_t currentTimestamp, uint32_t maxSilenceIntervalDS) {
    binary_semaphore_wait(nt->mutex);

    int prev = MAX_NEIGHBORS_NUM;  // Initialize as an invalid index
    int i = nt->firstNonDisconnected;

    while (i != MAX_NEIGHBORS_NUM) {
        if (meshTimeDiff(nt->neighbor[i].lastContactTimestamp, currentTimestamp) > maxSilenceIntervalDS) {
            int next = nt->neighbor[i].next;
            nt->neighbor[i].connectionState = DISCONNECTED;
            nt->connectedMask &= ~(1U << i);

            if (prev != MAX_NEIGHBORS_NUM) {
                nt->neighbor[prev].next = next;
            } else {
                nt->firstNonDisconnected = next;
            }
        }
        prev = i;
        i = nt->neighbor[i].next;
    }

    binary_semaphore_post(nt->mutex);
    return 0;
}

int getNthNeighbor(NeighborsTable * nt, int n, uint32_t * name, uint32_t *connectionState) {
	
	binary_semaphore_wait(nt->mutex);
	
    if (n < 0 || n >= MAX_NEIGHBORS_NUM) {
		binary_semaphore_post(nt->mutex);
        return -1;
    }

    *connectionState = nt->neighbor[n].connectionState;
    *name = nt->neighbor[n].name;
	
	binary_semaphore_post(nt->mutex);
    return 0;
}

int getNeighborsNamesList(NeighborsTable * nt, uint32_t neighbor[MAX_NEIGHBORS_NUM]){
	
	binary_semaphore_wait(nt->mutex);
	
	for (int i = 0; i < MAX_NEIGHBORS_NUM; i++) {
        if (nt->neighbor[i].connectionState != DISCONNECTED) {
            neighbor[i] = nt->neighbor[i].name;
        } else {
			neighbor[i] = 0U;
		}
    } 
	binary_semaphore_post(nt->mutex);
	return 0;
}

int setSelfMulticastBitNumber(NeighborsTable *nt, uint32_t name, uint8_t selfMulticastBitNumber) {

    binary_semaphore_wait(nt->mutex);
    
    int i = nt->firstNonDisconnected;

    while (i != MAX_NEIGHBORS_NUM) {
        if (nt->neighbor[i].connectionState != DISCONNECTED && nt->neighbor[i].name == name) {
            nt->neighbor[i].selfMulticastBitNumber = selfMulticastBitNumber;
            nt->neighbor[i].connectionState = CONNECTED;
            binary_semaphore_post(nt->mutex);
            return 0;
        }
        i = nt->neighbor[i].next;
    }

    binary_semaphore_post(nt->mutex);
    return -2;
}

int getSelfMulticastBitNumber(NeighborsTable *nt, uint32_t name, uint8_t *selfMulticastBitNumber) {

    binary_semaphore_wait(nt->mutex);

    int i = nt->firstNonDisconnected;

    while (i != MAX_NEIGHBORS_NUM) {
        if (nt->neighbor[i].connectionState == CONNECTED && nt->neighbor[i].name == name) {
            *selfMulticastBitNumber = nt->neighbor[i].selfMulticastBitNumber;
            binary_semaphore_post(nt->mutex);
            return 0;
        }
        i = nt->neighbor[i].next;
    }

    binary_semaphore_post(nt->mutex);
    return -2;
}

int getNeighborMask(NeighborsTable *nt, uint32_t name, uint32_t *mask) {

    binary_semaphore_wait(nt->mutex);

    int i = nt->firstNonDisconnected;

    while (i != MAX_NEIGHBORS_NUM) {
        if (nt->neighbor[i].connectionState != DISCONNECTED && nt->neighbor[i].name == name) {
            *mask = (1 << i);
            binary_semaphore_post(nt->mutex);
            return 0;
        }
        i = nt->neighbor[i].next;
    }

    binary_semaphore_post(nt->mutex);
    return -2;
}

int getConnectedMask(NeighborsTable * nt, uint32_t * connectedMask){
	
	binary_semaphore_wait(nt->mutex);
	*connectedMask = nt->connectedMask;
	binary_semaphore_post(nt->mutex);
	
	return 0;
}

int freeNeighborTable(NeighborsTable * nt) {
	
	binary_semaphore_destroy(nt->mutex);
	
    free(nt);
    return 0;
}

void printNeighborsTable(NeighborsTable *neighborsTable) {
    binary_semaphore_wait(neighborsTable->mutex);

    printf("Neighbors Table:\n");
    printf("%-20s%-20s%-20s%-20s\n", "Connection State", "Multicast Bit", "Name", "Last Contact Timestamp");

    int i = neighborsTable->firstNonDisconnected;

    while (i != MAX_NEIGHBORS_NUM) {
        printf("%-20u%-20u%-20u%-20u\n",
               neighborsTable->neighbor[i].connectionState,
               neighborsTable->neighbor[i].selfMulticastBitNumber,
               neighborsTable->neighbor[i].name,
               neighborsTable->neighbor[i].lastContactTimestamp);

        i = neighborsTable->neighbor[i].next;
    }

    binary_semaphore_post(neighborsTable->mutex);
}
