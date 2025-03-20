#include <InterestId.h>
#include <meshAPI.h>
#include <unistd.h> 

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MESH_ID 1

void app_main(){

	InterestId interestId;
    	makeInterestId(&interestId, INTEREST_ID_PART_1, INTEREST_ID_PART_2, INTEREST_ID_PART_3, INTEREST_ID_PART_4);
    
	uint8_t encrKey[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

  	mesh_network_handle_t *mnh = NULL;
  	joinNetwork(&mnh, MESH_ID, encrKey);
  	while(1){
  		vTaskDelay(100 / portTICK_PERIOD_MS);
  	}
	//leaveNetwork(&mnh);

}
