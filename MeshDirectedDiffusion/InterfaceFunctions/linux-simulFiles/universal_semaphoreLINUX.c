#include <stdlib.h>
#include <pthread.h>
#include "universal_semaphore.h"

struct binary_semaphore {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int value;
};

struct counting_semaphore {
    pthread_mutex_t mutex;
    int value;
};

binary_semaphore_t* binary_semaphore_create(int initial_value) {
    binary_semaphore_t* semaphore = (binary_semaphore_t*)malloc(sizeof(binary_semaphore_t));
    if (semaphore == NULL) {
        return NULL;
    }

    if (pthread_mutex_init(&(semaphore->mutex), NULL) != 0) {
        free(semaphore);
        return NULL;
    }

    if (pthread_cond_init(&(semaphore->cond), NULL) != 0) {
        pthread_mutex_destroy(&(semaphore->mutex));
        free(semaphore);
        return NULL;
    }

    semaphore->value = initial_value;
    return semaphore;
}

void binary_semaphore_destroy(binary_semaphore_t* semaphore) {
    pthread_mutex_destroy(&(semaphore->mutex));
    pthread_cond_destroy(&(semaphore->cond));
    free(semaphore);
}

void binary_semaphore_wait(binary_semaphore_t* semaphore) {
    pthread_mutex_lock(&(semaphore->mutex));
    while (semaphore->value <= 0) {
        pthread_cond_wait(&(semaphore->cond), &(semaphore->mutex));
    }
    semaphore->value--;
    pthread_mutex_unlock(&(semaphore->mutex));
}

int binary_semaphore_wait_timeoutMS(binary_semaphore_t* semaphore, int timeout_ms) {
    struct timespec timeout;
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_nsec = (timeout_ms % 1000) * 1000000;

    pthread_mutex_lock(&(semaphore->mutex));
    while (semaphore->value <= 0) {
        if (pthread_cond_timedwait(&(semaphore->cond), &(semaphore->mutex), &timeout) != 0) {
            pthread_mutex_unlock(&(semaphore->mutex));
            return 0; // Timeout occurred
        }
    }
    semaphore->value--;
    pthread_mutex_unlock(&(semaphore->mutex));
    return 1; // Semaphore acquired successfully
}

void binary_semaphore_post(binary_semaphore_t* semaphore) {
    pthread_mutex_lock(&(semaphore->mutex));
    semaphore->value++;
    pthread_cond_signal(&(semaphore->cond));
    pthread_mutex_unlock(&(semaphore->mutex));
}

counting_semaphore_t* counting_semaphore_create(int initial_count) {
    counting_semaphore_t* semaphore = (counting_semaphore_t*)malloc(sizeof(counting_semaphore_t));
    if (semaphore == NULL) {
        return NULL;
    }

    if (pthread_mutex_init(&(semaphore->mutex), NULL) != 0) {
        free(semaphore);
        return NULL;
    }

    semaphore->value = initial_count;
    return semaphore;
}

void counting_semaphore_destroy(counting_semaphore_t* semaphore) {
    pthread_mutex_destroy(&(semaphore->mutex));
    free(semaphore);
}

void counting_semaphore_wait(counting_semaphore_t* semaphore) {
    pthread_mutex_lock(&(semaphore->mutex));
    while (semaphore->value <= 0) {
        pthread_mutex_unlock(&(semaphore->mutex));
        pthread_mutex_lock(&(semaphore->mutex));
    }
    semaphore->value--;
    pthread_mutex_unlock(&(semaphore->mutex));
}

void counting_semaphore_post(counting_semaphore_t* semaphore) {
    pthread_mutex_lock(&(semaphore->mutex));
    semaphore->value++;
    pthread_mutex_unlock(&(semaphore->mutex));
}

void counting_semaphore_post_multiple(counting_semaphore_t* semaphore, int count) {
    pthread_mutex_lock(&(semaphore->mutex));
    semaphore->value += count;
    pthread_mutex_unlock(&(semaphore->mutex));
}
