#ifndef GRADIENTTABLE_H_
#define GRADIENTTABLE_H_

#include <stdbool.h>
#include <stdint.h>
#include <InterestId.h>
#include <universal_semaphore.h>
#include <meshSystemConfig.h>

typedef struct {
    InterestId interestId;
	uint8_t dataLevels[16]; //4 bits per neighbor, 32 neigbors * 4 bits = 16 bytes
	uint8_t interestsLevels[16]; //same as above
    uint32_t lastReinforcementTimestamp;
    uint32_t lastUniqueSendersNumber;
	uint8_t usnRepetitionsCounter;
	uint8_t next; 
} Gradient;

typedef struct {
    Gradient gradient[MAX_GRADIENTS_NUM];
    uint8_t firstUsed;
    binary_semaphore_t *mutex;
} GradientTable;

int initGradientTable(GradientTable **gt);
int gradientHandleInterestArrival(GradientTable *gt, InterestId interestId, uint32_t uniqueSendersNumber, uint32_t neighborBit, uint32_t timestamp, uint32_t * reinforceNeighborsMask);
int getGradient(GradientTable *gt, InterestId interestId, uint32_t *neighborsMask);
int gradientHandleNewDataArrival(GradientTable *gt, InterestId interestId, uint32_t neighborBit);
int gradientDecayInterestLevels(GradientTable *gt);
int refreshGradientTable(GradientTable *gt, uint32_t connectedMask, uint32_t currentTimestamp, uint32_t maxNotUsedIntervalDS);
int freeGradientTable(GradientTable *gt);

void printGradientTable(GradientTable *gradientTable);

#endif /* GRADIENTTABLE_H_ */
