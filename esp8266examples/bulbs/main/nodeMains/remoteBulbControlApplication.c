#include <meshAPI.h>
#include <stdlib.h>
#include <InterestId.h>
//#include "esp_log.h"
#include "FreeRTOS.h"
#include "task.h"
#include "esp_system.h"
#include "universal_semaphore.h"

#include <systemDependentHelpFunctions.h>
#include "bulbsControlSystem.h"

#define MESH_ID 1

#define MAX_DIST 5

#define BULBS_MAX 20

BulbInfo bulbsTable[BULBS_MAX];
int foundBulbsNum = 0;
binary_semaphore_t *mutex; 

void clearBulbsTable(){
	binary_semaphore_wait(mutex);
	foundBulbsNum = 0;
	binary_semaphore_post(mutex);
}

void receivedBulbInfoHandler(void *data, int dataLen){
	
    if (dataLen >= sizeof(BulbInfo)){
        BulbInfo bi;

        memcpy(&bi, data, sizeof(BulbInfo));
        
        binary_semaphore_wait(mutex);
        for (int i = 0; i < foundBulbsNum; ++i){
            if (compareInterestId(&bi.switchId, &bulbsTable[i].switchId) == 0){
                bulbsTable[i] = bi;
                binary_semaphore_post(mutex);
                return;
            }
        }
    
        if (foundBulbsNum < BULBS_MAX){
            bulbsTable[foundBulbsNum] = bi;
            foundBulbsNum++;
        }   
        binary_semaphore_post(mutex);
    }
}

int app_main(){
    mutex = binary_semaphore_create(1);

    InterestId recognitionId;
    makeInterestId(&recognitionId, RECOGNITION_ID_PART_1, RECOGNITION_ID_PART_2, RECOGNITION_ID_PART_3, RECOGNITION_ID_PART_4);
    
    mesh_network_handle_t *mnh;
    uint8_t encrKey[AES_KEY_LENGTH] = ENCRYPTION_KEY;
    
    joinNetwork(&mnh, MESH_ID, encrKey);
    
    int i = 0;
    SwitchRequest sr;
	
	subscribe(mnh, &recognitionId, MAX_DIST, receivedBulbInfoHandler);
	
    while(1){
        
        binary_semaphore_wait(mutex);
        if ( foundBulbsNum > 0 && i % 4 == 0 ){
            int randomBulb = rand() % foundBulbsNum;
            sr.sequenceNum = bulbsTable[randomBulb].sequenceNum;
            sr.randomField = rand();
            sendDataAlongGradient(mnh, &bulbsTable[randomBulb].switchId, (void*) &sr, sizeof(SwitchRequest));
        }
        binary_semaphore_post(mutex);
		
		if ( i % 40 == 0 ){
			clearBulbsTable();
		}
        
        ++i;
        delayProcess(1000);
    }
}
