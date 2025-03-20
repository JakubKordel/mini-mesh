#include "universal_semaphore.h"
#include "cmsis_os.h"          // Correct CMSIS-RTOS include
#include <stdlib.h>

typedef struct binary_semaphore {
    osSemaphoreId handle;
} binary_semaphore_t;

typedef struct counting_semaphore {
    osMutexId mutex;
    osSemaphoreId count_sem;
    int count;
} counting_semaphore_t;

// Create a binary semaphore with an initial value
binary_semaphore_t* binary_semaphore_create(int initial_value) {
    binary_semaphore_t* semaphore = (binary_semaphore_t*)malloc(sizeof(binary_semaphore_t));
    if (semaphore != NULL) {
        osSemaphoreDef(BinarySem);
        semaphore->handle = osSemaphoreCreate(osSemaphore(BinarySem), 1);
        if (semaphore->handle == NULL) {
            free(semaphore);
            return NULL;
        }
        if (initial_value == 0) {
            osSemaphoreWait(semaphore->handle, 0);  // Set to unavailable if initial value is 0
        }
    }
    return semaphore;
}

// Destroy a binary semaphore
void binary_semaphore_destroy(binary_semaphore_t* semaphore) {
    if (semaphore != NULL) {
        osSemaphoreDelete(semaphore->handle);
        free(semaphore);
    }
}

// Wait on a binary semaphore indefinitely
void binary_semaphore_wait(binary_semaphore_t* semaphore) {
    osSemaphoreWait(semaphore->handle, osWaitForever);
}

// Wait on a binary semaphore with a timeout
int binary_semaphore_wait_timeoutMS(binary_semaphore_t* semaphore, int timeout_ms) {
    return (osSemaphoreWait(semaphore->handle, timeout_ms) > 0) ? 1 : 0;
}

// Post (give) a binary semaphore
void binary_semaphore_post(binary_semaphore_t* semaphore) {
    osSemaphoreRelease(semaphore->handle);
}

// Create a counting semaphore with an initial count
counting_semaphore_t* counting_semaphore_create(int initial_count) {
    counting_semaphore_t* semaphore = (counting_semaphore_t*)malloc(sizeof(counting_semaphore_t));
    if (semaphore != NULL) {
        osMutexDef(Mutex);
        semaphore->mutex = osMutexCreate(osMutex(Mutex));
        if (semaphore->mutex == NULL) {
            free(semaphore);
            return NULL;
        }

        osSemaphoreDef(CountSem);
        semaphore->count_sem = osSemaphoreCreate(osSemaphore(CountSem), initial_count);
        if (semaphore->count_sem == NULL) {
            osMutexDelete(semaphore->mutex);
            free(semaphore);
            return NULL;
        }
        semaphore->count = initial_count;
    }
    return semaphore;
}

// Destroy a counting semaphore
void counting_semaphore_destroy(counting_semaphore_t* semaphore) {
    if (semaphore != NULL) {
        osSemaphoreDelete(semaphore->count_sem);
        osMutexDelete(semaphore->mutex);
        free(semaphore);
    }
}

// Wait on a counting semaphore (decrement the count)
void counting_semaphore_wait(counting_semaphore_t* semaphore) {
    osMutexWait(semaphore->mutex, osWaitForever);
    while (semaphore->count <= 0) {
        osMutexRelease(semaphore->mutex);
        osSemaphoreWait(semaphore->count_sem, osWaitForever);
        osMutexWait(semaphore->mutex, osWaitForever);
    }
    semaphore->count--;
    osMutexRelease(semaphore->mutex);
}

// Post (increment) a counting semaphore
void counting_semaphore_post(counting_semaphore_t* semaphore) {
    osMutexWait(semaphore->mutex, osWaitForever);
    semaphore->count++;
    if (semaphore->count > 0) {
        osSemaphoreRelease(semaphore->count_sem);
    }
    osMutexRelease(semaphore->mutex);
}

// Post (increment) a counting semaphore multiple times
void counting_semaphore_post_multiple(counting_semaphore_t* semaphore, int count) {
    osMutexWait(semaphore->mutex, osWaitForever);
    semaphore->count += count;
    while (count-- > 0) {
        osSemaphoreRelease(semaphore->count_sem);
    }
    osMutexRelease(semaphore->mutex);
}
