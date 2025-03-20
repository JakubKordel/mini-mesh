#include "universal_semaphore.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

typedef struct counting_semaphore {
    SemaphoreHandle_t mutex;     
    SemaphoreHandle_t count_sem;  
    int count;
} counting_semaphore_t;

struct binary_semaphore {
    SemaphoreHandle_t handle;
};

binary_semaphore_t* binary_semaphore_create(int initial_value) {
    binary_semaphore_t* semaphore = (binary_semaphore_t*)malloc(sizeof(binary_semaphore_t));
    if (semaphore) {
        semaphore->handle = xSemaphoreCreateBinary();
        if (initial_value) {
            xSemaphoreGive(semaphore->handle);
        }
    }
    return semaphore;
}

void binary_semaphore_destroy(binary_semaphore_t* semaphore) {
    if (semaphore) {
        vSemaphoreDelete(semaphore->handle);
        free(semaphore);
    }
}

void binary_semaphore_wait(binary_semaphore_t* semaphore) {
    xSemaphoreTake(semaphore->handle, portMAX_DELAY);
}

int binary_semaphore_wait_timeoutMS(binary_semaphore_t* semaphore, int timeout_ms) {
    TickType_t timeout_ticks = pdMS_TO_TICKS(timeout_ms);
    if (xSemaphoreTake(semaphore->handle, timeout_ticks) == pdTRUE) {
        return 1; 
    } else {
        return 0; 
    }
}

void binary_semaphore_post(binary_semaphore_t* semaphore) {
    xSemaphoreGive(semaphore->handle);
}

counting_semaphore_t* counting_semaphore_create(int initial_count) {
    counting_semaphore_t* semaphore = (counting_semaphore_t*)malloc(sizeof(counting_semaphore_t));
    semaphore->mutex = xSemaphoreCreateMutex();
    semaphore->count_sem = binary_semaphore_create(0);
    semaphore->count = initial_count;

    return semaphore;
}

void counting_semaphore_destroy(counting_semaphore_t* semaphore) {
    binary_semaphore_destroy(semaphore->count_sem);
    vSemaphoreDelete(semaphore->mutex);
    free(semaphore);
}

void counting_semaphore_wait(counting_semaphore_t* semaphore) {
    xSemaphoreTake(semaphore->mutex, portMAX_DELAY);

    while (semaphore->count <= 0) {
        xSemaphoreGive(semaphore->mutex);  
        binary_semaphore_wait(semaphore->count_sem);
        xSemaphoreTake(semaphore->mutex, portMAX_DELAY); 
    }

    semaphore->count--;
    xSemaphoreGive(semaphore->mutex);
}

void counting_semaphore_post(counting_semaphore_t* semaphore) {
    xSemaphoreTake(semaphore->mutex, portMAX_DELAY);

    semaphore->count++;
    if (semaphore->count == 1) {
        binary_semaphore_post(semaphore->count_sem);
    }

    xSemaphoreGive(semaphore->mutex);
}