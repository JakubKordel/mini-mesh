#include "gradientTable.h"
#include <meshLibraryProcesses.h>
#include <meshSystemConfig.h>

int initGradientTable(GradientTable **gt) {
    *gt = (GradientTable*)malloc(sizeof(GradientTable));
    if (*gt == NULL) {
        return -1;
    }

    (*gt)->firstUsed = MAX_GRADIENTS_NUM; 
    (*gt)->mutex = binary_semaphore_create(1);

    return 0;
}

int gradientHandleInterestArrival(GradientTable *gt, InterestId interestId, uint32_t uniqueSendersNumber, uint32_t neighborBit, uint32_t timestamp, uint32_t * reinforceNeighborsMask) {
	
	binary_semaphore_wait(gt->mutex);
	
	*reinforceNeighborsMask = 0U;
	
	int emptySlot = MAX_GRADIENTS_NUM;
	int i = gt->firstUsed;
	
	int byte = 0;
	uint32_t bit = neighborBit;
	int half = 0;
	
	if (neighborBit != 0U){
		while (bit != 1U ){
			byte++;
			bit = bit >> 1;
		}
		half = byte % 2;
		byte = byte / 2;
	}
	
	if (gt->firstUsed > 0){
		emptySlot = 0;
	}
	
    while (i != MAX_GRADIENTS_NUM) {
        if (compareInterestId(&gt->gradient[i].interestId, &interestId) == 0) {
			uint8_t currentVal = 0U;
			if (half == 0){
				currentVal = gt->gradient[i].interestsLevels[byte] & 0b00001111;
			} else {
				currentVal = (gt->gradient[i].interestsLevels[byte] & 0b11110000 ) >> 4;
			}
			
            if (gt->gradient[i].lastUniqueSendersNumber != uniqueSendersNumber) {
                gt->gradient[i].lastUniqueSendersNumber = uniqueSendersNumber;
                gt->gradient[i].lastReinforcementTimestamp = timestamp;
				gt->gradient[i].usnRepetitionsCounter = 0;
				currentVal += 3U;
				if (currentVal > 15U ){
					currentVal = 15U;
				}
				
				uint32_t bitc = 1U;
				for (int j = 0; j < 32 ; ++j){
					int halfc = j % 2;
					int bytec = j / 2;
					if ((halfc == 0 && (gt->gradient[i].dataLevels[bytec] & 0b00001111) > 0U ) 
										|| (halfc != 0 && (gt->gradient[i].dataLevels[bytec] & 0b11110000) > 0U )){
						*reinforceNeighborsMask |= bitc;
					}
					
					bitc = bitc << 1;
				}
				if (*reinforceNeighborsMask == 0U ){
					
					*reinforceNeighborsMask = 0xFFFFFFFF & ~neighborBit;
					
				}
				
            } else {
				if (gt->gradient[i].usnRepetitionsCounter != 255U){
					gt->gradient[i].usnRepetitionsCounter++;
				}
				if (gt->gradient[i].usnRepetitionsCounter == 1){
					currentVal += 2U;
				} else if (gt->gradient[i].usnRepetitionsCounter == 2) {
					currentVal += 1U;
				}
				if (currentVal > 15U ){
					currentVal = 15U;
				}
			}
			
			if (neighborBit != 0U){
				if (half == 0){
					gt->gradient[i].interestsLevels[byte] &= 0b11110000;
					gt->gradient[i].interestsLevels[byte] |= currentVal;
				} else {
					gt->gradient[i].interestsLevels[byte] &= 0b00001111;
					gt->gradient[i].interestsLevels[byte] |= (currentVal << 4);
				}
			}
			
			binary_semaphore_post(gt->mutex);
            return 0;
        } 
		
		if (emptySlot == MAX_GRADIENTS_NUM && gt->gradient[i].next - i > 1) {
            emptySlot = i + 1;
        }

        i = gt->gradient[i].next;
    }
				
	if (emptySlot != MAX_GRADIENTS_NUM){
		if (emptySlot != 0) {
			gt->gradient[emptySlot].next = gt->gradient[emptySlot-1].next;
			gt->gradient[emptySlot-1].next = emptySlot;
		} else {
			gt->gradient[emptySlot].next = gt->firstUsed;
			gt->firstUsed = 0;
		}
		
		memcpy(&gt->gradient[emptySlot].interestId, &interestId, sizeof(InterestId));
        gt->gradient[emptySlot].lastUniqueSendersNumber = uniqueSendersNumber;
		

		for (int i = 0; i < 16 ; ++i){
			gt->gradient[emptySlot].interestsLevels[i] = 0U;
			gt->gradient[emptySlot].dataLevels[i] = 0U;
		}
			
		if (neighborBit != 0U){
			if (half == 0){
				gt->gradient[emptySlot].interestsLevels[byte] = 3U;
			} else {
				gt->gradient[emptySlot].interestsLevels[byte] = 18U;
			}
		}
        gt->gradient[emptySlot].lastReinforcementTimestamp = timestamp;
		gt->gradient[emptySlot].usnRepetitionsCounter = 0;
		*reinforceNeighborsMask = 0xFFFFFFFF & ~neighborBit;
		binary_semaphore_post(gt->mutex);
        return 0;
	}

	binary_semaphore_post(gt->mutex);
    return -1; 
}

int getGradient(GradientTable *gt, InterestId interestId, uint32_t *neighborsMask) {

    if (gt == NULL) {
        return -1;
    }

    binary_semaphore_wait(gt->mutex);
    *neighborsMask = 0U;

    int i = gt->firstUsed;

    while (i != MAX_GRADIENTS_NUM) {
        if (compareInterestId(&gt->gradient[i].interestId, &interestId) == 0) {
			uint32_t bit = 1U;
			for (int j = 0; j < 32 ; ++j){
				int half = j % 2;
				int byte = j / 2;
				if ((half == 0 && (gt->gradient[i].interestsLevels[byte] & 0b00001111) > 0U ) 
										|| (half != 0 && (gt->gradient[i].interestsLevels[byte] & 0b11110000) > 0U )){
					*neighborsMask |= bit;
				}
				bit = bit << 1;
			}
			
			break;
        }

        i = gt->gradient[i].next;
    }

    binary_semaphore_post(gt->mutex);

    return 0;
}

int gradientHandleNewDataArrival(GradientTable *gt, InterestId interestId, uint32_t neighborBit){
	if (neighborBit == 0U){
		return -2;
	}
	
	binary_semaphore_wait(gt->mutex);
	
	int i = gt->firstUsed;
	
	int byte = 0;
	uint32_t bit = neighborBit;
	while (bit != 1U ){
		byte++;
		bit = bit >> 1;
	}
	int half = byte % 2;
	byte = byte / 2;
	
    while (i != MAX_GRADIENTS_NUM) {
        if (compareInterestId(&gt->gradient[i].interestId, &interestId) == 0) {
			uint8_t currentVal;
			if (half == 0){
				currentVal = gt->gradient[i].dataLevels[byte] & 0b00001111;
			} else {
				currentVal = (gt->gradient[i].dataLevels[byte] & 0b11110000 ) >> 4;
			}
			
			currentVal += 1U;
			if (currentVal > 15U ){
				currentVal = 15U;
			}
			
			if (half == 0){
				gt->gradient[i].dataLevels[byte] &= 0b11110000;
				gt->gradient[i].dataLevels[byte] |= currentVal;
			} else {
				gt->gradient[i].dataLevels[byte] &= 0b00001111;
				gt->gradient[i].dataLevels[byte] |= (currentVal << 4);
			}
			
			binary_semaphore_post(gt->mutex);
            return 0;
        } 

        i = gt->gradient[i].next;
    }
	
	binary_semaphore_post(gt->mutex);
    return -1; 
}

int gradientDecayInterestLevels(GradientTable *gt){
	binary_semaphore_wait(gt->mutex);
	
	int i = gt->firstUsed;
    	int prev = MAX_GRADIENTS_NUM;

    	while (i != MAX_GRADIENTS_NUM) {

        	prev = i;
        	i = gt->gradient[i].next;
    	}


	binary_semaphore_post(gt->mutex);
	return 0;
}

int refreshGradientTable(GradientTable *gt, uint32_t connectedMask, uint32_t currentTimestamp, uint32_t maxNotUsedIntervalDS) {

    binary_semaphore_wait(gt->mutex);

    int i = gt->firstUsed;
    int prev = MAX_GRADIENTS_NUM;

    while (i != MAX_GRADIENTS_NUM) {
        if ((meshTimeDiff(gt->gradient[i].lastReinforcementTimestamp, currentTimestamp) > maxNotUsedIntervalDS)) {

            if (prev != MAX_GRADIENTS_NUM) {
                gt->gradient[prev].next = gt->gradient[i].next;
            } else {
                gt->firstUsed = gt->gradient[i].next;
            }
        }

        prev = i;
        i = gt->gradient[i].next;
    }

    binary_semaphore_post(gt->mutex);
    return 0;
}

int freeGradientTable(GradientTable * gt) {
	
	binary_semaphore_destroy(gt->mutex);
    free(gt);

    return 0;
}

#include <stdio.h>


void printGradientTable(GradientTable *gradientTable) {
    binary_semaphore_wait(gradientTable->mutex);

    printf("Gradient Table (In Use):\n");
    printf("%-20s%-35s%-35s%-20s%-20s%-20s\n", "InterestId", "Data Levels", "Interest Levels", "Last Reinforcement Timestamp", "Last Unique Senders Number", "USN Repetitions Counter");

    uint32_t i = gradientTable->firstUsed;
	
    while (i != MAX_GRADIENTS_NUM) {
        printf("%08X%08X%08X%08X%-35s", gradientTable->gradient[i].interestId.id[0],
               gradientTable->gradient[i].interestId.id[1], gradientTable->gradient[i].interestId.id[2],
               gradientTable->gradient[i].interestId.id[3], "");  

        for (int j = 0; j < 16; ++j) {
            printf("%02X ", gradientTable->gradient[i].dataLevels[j]);  
        }
        printf("%-35s", "");

        for (int j = 0; j < 16; ++j) {
            printf("%02X ", gradientTable->gradient[i].interestsLevels[j]);  
        }

        printf("%-20u%-20u%-20u\n",
               gradientTable->gradient[i].lastReinforcementTimestamp,
               gradientTable->gradient[i].lastUniqueSendersNumber,
               gradientTable->gradient[i].usnRepetitionsCounter);

        i = gradientTable->gradient[i].next;
    }

    binary_semaphore_post(gradientTable->mutex);
}
