#include "systemDependentHelpFunctions.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

ProcessHandle startProcess(ProcessFunction processFunction, const char *processName, uint32_t stackSize, void *parameters, uint32_t priority) {
    TaskHandle_t taskHandle;
    BaseType_t result = xTaskCreate(processFunction, processName, stackSize, parameters, priority, &taskHandle);
    return (result == pdPASS) ? taskHandle : NULL;
}

void stopProcess(ProcessHandle processHandle) {
	vTaskDelete(processHandle);
}

void delayProcess(int timeMs) {
    TickType_t delayTicks = pdMS_TO_TICKS(timeMs);
    vTaskDelay(delayTicks);
}

void getMACAddress(uint8_t *mac) {
    esp_efuse_mac_get_default(mac);
}

void generateRandomNodeName(uint32_t *nodeName) {
	
	uint8_t mac[6];
    getMACAddress(mac);
    uint32_t last4Bytes;
	memcpy(&last4Bytes ,&mac[2], sizeof(uint32_t));
	srand(last4Bytes);  
	
    *nodeName = (uint32_t)rand();
}
