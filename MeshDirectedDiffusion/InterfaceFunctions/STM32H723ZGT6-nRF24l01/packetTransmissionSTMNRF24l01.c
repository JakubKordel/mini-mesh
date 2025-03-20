#include "packetTransmission.h"
#include "cmsis_os.h"
#include "stm32h7xx_hal.h"
#include "driver_nrf24l01_interface.h"
#include "nrf24l01meshDriver.h"
#include "meshLibraryProcesses.h"
#include "systemDependentHelpFunctions.h"
#include "universal_semaphore.h"
#include <stdio.h>

#define NRF_IRQ_PIN     GPIO_PIN_12
#define NRF_IRQ_PORT    GPIOD

#define MAX_PAYLOAD_SIZE 32

static MeshSystem *mSystem = NULL;
osMutexId nrf_mutex;
osSemaphoreId irqSemaphore;

void receivedDataCallback(uint8_t *data, int length) {
    osMutexWait(nrf_mutex, osWaitForever);
    packetReceivedCallback(mSystem, data, length);
    osMutexRelease(nrf_mutex);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == NRF_IRQ_PIN) {
        osSemaphoreRelease(irqSemaphore);
    }
}

void nrf24_receive_task(void const *argument) {
    while (1) {
        osSemaphoreWait(irqSemaphore, osWaitForever);
        osMutexWait(nrf_mutex, osWaitForever);
        nrf24l01MeshInteruptHandler();
        osMutexRelease(nrf_mutex);
    }
}

int initTransmissionFunctionality(MeshSystem *meshSystem) {
    mSystem = meshSystem;

    osMutexDef(nrf_mutex);
    nrf_mutex = osMutexCreate(osMutex(nrf_mutex));

    osSemaphoreDef(irqSemaphore);
    irqSemaphore = osSemaphoreCreate(osSemaphore(irqSemaphore), 0);

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = NRF_IRQ_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(NRF_IRQ_PORT, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(EXTI2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI2_IRQn);

    nrf24l01MeshSetReceiveCallback(receivedDataCallback);
    osDelay(500);
    nrf24l01MeshInit();
    nrf24l01MeshTypeSwitch(NRF24L01_TYPE_RX);

    osThreadDef(nrf24_receive, nrf24_receive_task, osPriorityHigh, 0, 4096);
    osThreadCreate(osThread(nrf24_receive), NULL);
    return 0;
}

int transmit_data(MeshSystem *meshSystem, uint8_t *packetPtr, int length) {
    int chunkSize = 30;
    int numChunks = (length + chunkSize - 1) / chunkSize;

    osMutexWait(nrf_mutex, osWaitForever);

    nrf24l01MeshTypeSwitch(NRF24L01_TYPE_TX);
    nrf24l01MeshSetInactive();
    osDelay(1);
    nrf24l01MeshSetActive();

    for (int partNum = 0; partNum < numChunks; partNum++) {
        int offset = partNum * chunkSize;
        int remainingBytes = length - offset;
        int currentChunkSize = (remainingBytes > chunkSize) ? chunkSize : remainingBytes;

        uint8_t chunkBuffer[currentChunkSize + 2];
        chunkBuffer[0] = (uint8_t)partNum;
        chunkBuffer[1] = (uint8_t)currentChunkSize;

        memcpy(&chunkBuffer[2], &packetPtr[offset], currentChunkSize);
        nrf24l01MeshWritePayloadNoAck(chunkBuffer, (uint8_t)currentChunkSize + 2);

        osDelay(1);
        nrf24l01MeshSetInactive();
        osDelay(1);
        nrf24l01MeshSetActive();
    }

    uint8_t endPacket[2] = {numChunks, 0};
    nrf24l01MeshWritePayloadNoAck(endPacket, 2);
    osDelay(10);
    nrf24l01MeshSetInactive();
    osDelay(1);
    nrf24l01MeshSetActive();

    nrf24l01MeshSetInactive();
    nrf24l01MeshTypeSwitch(NRF24L01_TYPE_RX);
    nrf24l01MeshSetActive();

    osMutexRelease(nrf_mutex);

    return 0;
}

int deinitTransmissionFunctionality(MeshSystem *meshSystem) {
    nrf24l01MeshDeinit();
    return 0;
}
