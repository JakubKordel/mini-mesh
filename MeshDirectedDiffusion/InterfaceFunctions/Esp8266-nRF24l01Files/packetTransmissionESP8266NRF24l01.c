#include "packetTransmission.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "driver/gpio.h"
#include "esp8266/gpio_register.h"
#include "esp8266/pin_mux_register.h"
#include "esp8266/eagle_soc.h"
#include <rom/ets_sys.h>

#include <nrf24l01meshDriver.h>
#include <meshLibraryProcesses.h>
#include <systemDependentHelpFunctions.h>

#include <universal_semaphore.h>

#define GPIO_IRQ_PIN 4
#define MAX_PAYLOAD_SIZE 32

binary_semaphore_t * nrf_mutex = NULL;

static MeshSystem *mSystem = NULL;
uint8_t res;

SemaphoreHandle_t irqSemaphore;

void receivedDataCallback(uint8_t *data, int length) {
    taskEXIT_CRITICAL();
	packetReceivedCallback(mSystem, data, length);
	taskENTER_CRITICAL();
	
}

void IRAM_ATTR irq_handler(void *arg) {
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(irqSemaphore, &xHigherPriorityTaskWoken);
	if(xHigherPriorityTaskWoken){
		portYIELD_FROM_ISR();
	}
}

void nrf24_receive_task(void *pvParameters) {

	while (1) {
		if (xSemaphoreTake(irqSemaphore, portMAX_DELAY)) {
			taskENTER_CRITICAL();
            nrf24l01MeshInteruptHandler();
            taskEXIT_CRITICAL();
		}
	}
    vTaskDelete(NULL);
}

int initTransmissionFunctionality(MeshSystem *meshSystem) {
    mSystem = meshSystem;
	
	nrf_mutex = binary_semaphore_create(1);
	irqSemaphore = xSemaphoreCreateBinary();
	
	gpio_config_t gpio_cfg = {
		.pin_bit_mask = (1ULL << GPIO_NUM_2),
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_ANYEDGE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };

    gpio_config(&gpio_cfg);
	
	gpio_install_isr_service(0);
	gpio_isr_handler_add(GPIO_NUM_2, irq_handler, (void*)GPIO_NUM_2);

	nrf24l01MeshSetReceiveCallback(receivedDataCallback);
	nrf24l01MeshInit();
	nrf24l01MeshTypeSwitch(NRF24L01_TYPE_RX);
	
	xTaskCreate(nrf24_receive_task, "nrf24_receive_task", 4096, NULL, configMAX_PRIORITIES - 1, NULL);

    return 0;
}

int transmit_data(MeshSystem *meshSystem, uint8_t *packetPtr, int length) {

    int chunkSize = 30;  
    int numChunks = (length + chunkSize - 1) / chunkSize; 
	
	binary_semaphore_wait(nrf_mutex);
	
	taskENTER_CRITICAL();
	
	nrf24l01MeshTypeSwitch(NRF24L01_TYPE_TX);
	
	nrf24l01MeshSetInactive();
	ets_delay_us(10);
	nrf24l01MeshSetActive();
	
    for (int partNum = 0; partNum < numChunks; partNum++) {
        int offset = partNum * chunkSize;
        int remainingBytes = length - offset;
        int currentChunkSize = (remainingBytes > chunkSize) ? chunkSize : remainingBytes;

        uint8_t chunkBuffer[currentChunkSize + 2];

        chunkBuffer[0] = (uint8_t)partNum;
        chunkBuffer[1] = (uint8_t)currentChunkSize;

        memcpy(&chunkBuffer[2], &packetPtr[offset], currentChunkSize);
        nrf24l01MeshWritePayloadNoAck(chunkBuffer, (uint8_t) currentChunkSize + 2);
		
		ets_delay_us(100);
		nrf24l01MeshSetInactive();
		ets_delay_us(10);		
		nrf24l01MeshSetActive();
    }
	
	uint8_t endPacket[2] = {numChunks, 0};
    nrf24l01MeshWritePayloadNoAck(endPacket, (uint8_t)2);
	ets_delay_us(1000);
	nrf24l01MeshSetInactive();
	ets_delay_us(10);
	nrf24l01MeshSetActive();

	nrf24l01MeshSetInactive();
	
	nrf24l01MeshTypeSwitch(NRF24L01_TYPE_RX);
	nrf24l01MeshSetActive();
	
	taskEXIT_CRITICAL();
		
	binary_semaphore_post(nrf_mutex);
	
    return 0;
}

int deinitTransmissionFunctionality(MeshSystem *meshSystem) {

	nrf24l01MeshDeinit();

    return 0;
}