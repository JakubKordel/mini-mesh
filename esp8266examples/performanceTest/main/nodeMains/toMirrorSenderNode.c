#include <InterestId.h>
#include <meshAPI.h>

#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

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

#define MSG_SIZE 100
#define MSGS_NUM 100

bool firstReceived = false;
TickType_t firstMsgTick, lastMsgTick;
int receivedBytesCounter = 0;

void receivedMirroredDataHandler(void *data, int size){
    int msgIndex;
    memcpy(&msgIndex, data, sizeof(int));
    ESP_LOGI("APP_MAIN", "Msg Index: %d", msgIndex);
    
    lastMsgTick = xTaskGetTickCount();

    if (!firstReceived){
        firstMsgTick = lastMsgTick;
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

void app_main(){

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
    
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    
    for (int i = 0; i < MSGS_NUM; i++){ 
        //mirror msg index (bytes of int i) and place on the end of msg
        for (int j = 0; j < sizeof(int); j++) {
            msg[MSG_SIZE - sizeof(int) + j] = *(((char*)(&i)) + sizeof(int) - 1 - j) ;
    }
        
        sendDataAlongGradient(mnh, &mirrorId, msg, MSG_SIZE);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    
    while (1){
        long elapsed_ms = (lastMsgTick - firstMsgTick) * portTICK_PERIOD_MS;

        ESP_LOGI("APP_MAIN", "Elapsed time: %ld milliseconds", elapsed_ms);
        ESP_LOGI("APP_MAIN", "Received Bytes: %d", receivedBytesCounter);
        
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
