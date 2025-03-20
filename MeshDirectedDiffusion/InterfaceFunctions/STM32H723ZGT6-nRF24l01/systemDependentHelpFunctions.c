#include "systemDependentHelpFunctions.h"
#include "cmsis_os.h"          // Correct include for CMSIS-RTOS
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_rng.h"
#include "stm32h7xx_hal_tim.h"
#include <stdlib.h>
#include <string.h>

// External declaration of RNG handle (ensure it is declared and initialized elsewhere in the code)
extern RNG_HandleTypeDef hrng;

static uint8_t rng_initialized = 0;

static void initRandomSeed() {
    uint32_t rng_value;

    if (rng_initialized) {
        // Check if RNG is successfully generating random numbers
        if (HAL_RNG_GenerateRandomNumber(&hrng, &rng_value) == HAL_OK) {
            srand(rng_value);  // Use random value from RNG as seed for srand
        } else {
            srand(0x12345678);  // Fallback seed if RNG fails
        }
    } else {
        srand(0x12345678);  // Use fixed seed if RNG not initialized
    }
}

// Function to start a CMSIS-RTOS task (Process)
ProcessHandle startProcess(ProcessFunction processFunction, const char *processName, uint32_t stackSize, void *parameters, uint32_t priority) {
    osThreadId taskHandle;
    osThreadDef_t taskDef = { (char*)processName, (os_pthread)processFunction, (osPriority)priority, 0, stackSize };

    taskHandle = osThreadCreate(&taskDef, parameters);
    return (taskHandle != NULL) ? taskHandle : NULL;  // Return task handle if successful
}

// Function to stop a CMSIS-RTOS task
void stopProcess(ProcessHandle processHandle) {
    if (processHandle != NULL) {
        osThreadTerminate(processHandle);  // Terminate the task
    }
}

// Function to delay the current process/task for a specified time in ms
void delayProcess(int timeMs) {
    osDelay(timeMs);  // CMSIS-RTOS function to delay task
}

// Function to generate a random node name (using random number generation)
void generateRandomNodeName(uint32_t *nodeName) {
    if (nodeName != NULL) {
        initRandomSeed();  // Initialize RNG seed
        *nodeName = (uint32_t)rand();  // Assign random value to node name
    }
}

// Function to initialize the RNG (Random Number Generator)
void initializeRNG(void) {
    if (HAL_RNG_Init(&hrng) == HAL_OK) {  // Initialize RNG
        rng_initialized = 1;  // Mark RNG as initialized
    } else {
        rng_initialized = 0;  // Mark RNG as not initialized in case of failure
    }
}
