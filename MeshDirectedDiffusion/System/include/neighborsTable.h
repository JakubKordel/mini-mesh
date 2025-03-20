#ifndef NEIGHBORSTABLE_H_
#define NEIGHBORSTABLE_H_

#include <stdbool.h>
#include <stdint.h>
#include <universal_semaphore.h>
#include <meshSystemConfig.h>

//connection states
#define DISCONNECTED		0b00000000
#define NO_MULTICAST_BIT 	0b00000010
#define CONNECTED			0b00000001	

typedef struct {
    uint8_t connectionState;
    uint8_t selfMulticastBitNumber;
    uint32_t name;
    uint32_t lastContactTimestamp;
    int next;
} Neighbor;

typedef struct {
    Neighbor neighbor[MAX_NEIGHBORS_NUM];
	uint32_t connectedMask;
	int firstNonDisconnected;
	binary_semaphore_t * mutex;
} NeighborsTable; 

int initNeighborTable(NeighborsTable ** nt);
int neighborHeardHandle(NeighborsTable * nt, uint32_t name, uint32_t connectionTimestamp);
int disconnectSilentNeighbors(NeighborsTable * nt, uint32_t currentTimestamp, uint32_t maxSilenceIntervalDS);
int getNthNeighbor(NeighborsTable * nt, int n, uint32_t * name, uint32_t * connectionState);
int getNeighborsNamesList(NeighborsTable * nt, uint32_t neighbor[MAX_NEIGHBORS_NUM]);
int getNeighborMask(NeighborsTable * nt, uint32_t name, uint32_t* mask);
int setSelfMulticastBitNumber(NeighborsTable * nt, uint32_t name, uint8_t selfMulticastBitNumber);
int getSelfMulticastBitNumber(NeighborsTable * nt, uint32_t name, uint8_t * selfMulticastBitNumber);
int getConnectedMask(NeighborsTable * nt, uint32_t * connectedMask);
int freeNeighborTable(NeighborsTable * nt);

void printNeighborsTable(NeighborsTable *neighborsTable);

#endif /* NEIGHBORSTABLE_H_ */


