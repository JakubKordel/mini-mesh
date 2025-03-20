#include <meshAPI.h>
#include <InterestId.h>
// #include "esp_log.h"
#include "FreeRTOS.h"
#include "task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "universal_semaphore.h"

#include <systemDependentHelpFunctions.h>
#include "bulbsControlSystem.h"

#define MESH_ID 1

#define BULB_NAME 1

#define SWITCH_ID_PART_1 0x00000000
#define SWITCH_ID_PART_2 0x00000000
#define SWITCH_ID_PART_3 0x00000000
#define SWITCH_ID_PART_4 0x00000002

#define MAX_DIST 5

#define BULB_PIN 5 // D1 on NodeMCU

binary_semaphore_t *mutex; 

BulbInfo bulbInfo;

mesh_network_handle_t *mnh;

void configBulb(){
    gpio_config_t io_conf;
    io_conf.pin_bit_mask = (1ULL << BULB_PIN); // Configure the specific GPIO pin
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);
}

void turnOffBulb(){
	gpio_set_level(BULB_PIN, 0);
} 

void turnOnBulb(){
	gpio_set_level(BULB_PIN, 1);
}

void receivedRecognitionInterestHandler(void * params){
	
}

void receivedSwitchRequestHandler(void *data, int dataLen){
	
	if (dataLen >= sizeof(SwitchRequest)){
		SwitchRequest req;
		memcpy(&req, data, sizeof(SwitchRequest));
		
		binary_semaphore_wait(mutex);
		if (req.sequenceNum == bulbInfo.sequenceNum){
			bulbInfo.sequenceNum++;
			if (bulbInfo.state){
				bulbInfo.state = false;
				turnOffBulb();
			} else {
				bulbInfo.state = true;
				turnOnBulb();
			}
			InterestId recognitionId;
			makeInterestId(&recognitionId, RECOGNITION_ID_PART_1, RECOGNITION_ID_PART_2, RECOGNITION_ID_PART_3, RECOGNITION_ID_PART_4);
			
			sendDataAlongGradient(mnh, &recognitionId, (void*) &bulbInfo, sizeof(BulbInfo));
		}
		binary_semaphore_post(mutex); 
	}
}

int app_main(){
	
	mutex = binary_semaphore_create(1);
	
	configBulb();
	
	InterestId switchId;
    	makeInterestId(&switchId, SWITCH_ID_PART_1, SWITCH_ID_PART_2, SWITCH_ID_PART_3, SWITCH_ID_PART_4);
	
	InterestId recognitionId;
	makeInterestId(&recognitionId, RECOGNITION_ID_PART_1, RECOGNITION_ID_PART_2, RECOGNITION_ID_PART_3, RECOGNITION_ID_PART_4);
	
	bulbInfo.name = BULB_NAME;
	bulbInfo.state = false;
	bulbInfo.switchId = switchId;
	bulbInfo.sequenceNum = 0;
	
    	uint8_t encrKey[AES_KEY_LENGTH] = ENCRYPTION_KEY;
	
   	joinNetwork(&mnh, MESH_ID, encrKey);
	publish(mnh, &recognitionId, receivedRecognitionInterestHandler, (void*) mnh);
	subscribe(mnh, &switchId, MAX_DIST, receivedSwitchRequestHandler); 
	
	while (1){
	
		binary_semaphore_wait(mutex);
		BulbInfo bi = bulbInfo;
		binary_semaphore_post(mutex);
		
		bi.randomField = rand();
	
		sendDataAlongGradient(mnh, &recognitionId, (void*) &bi, sizeof(BulbInfo));
		
		delayProcess(250);
	}
}
