#include <InterestId.h>
#include <meshAPI.h>
#include <unistd.h> 

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <systemDependentHelpFunctions.h>
#include "bulbsControlSystem.h"

#define MESH_ID 1

void app_main(){
    
	uint8_t encrKey[AES_KEY_LENGTH] = ENCRYPTION_KEY;

  	mesh_network_handle_t *mnh = NULL;
  	joinNetwork(&mnh, MESH_ID, encrKey);
  	while(1){
  		vTaskDelay(100 / portTICK_PERIOD_MS);
  	}
	//leaveNetwork(&mnh);

}
