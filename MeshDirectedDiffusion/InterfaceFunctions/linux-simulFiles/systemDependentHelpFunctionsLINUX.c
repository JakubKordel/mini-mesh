#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h> 
#include <stdint.h> 
#include <sys/types.h> 
#include <pthread.h>
#include "systemDependentHelpFunctions.h"

// Define the process handle structure
typedef struct {
    pthread_t thread_id;
} ProcessInfo;

ProcessHandle startProcess(void (*processFunction)(void *), const char *processName, uint32_t stackSize, void *parameters, uint32_t priority) {
    // Create a thread for the process
    pthread_t thread;
    if (pthread_create(&thread, NULL, (void *(*)(void *))processFunction, parameters) != 0) {
        fprintf(stderr, "Failed to create thread for process\n");
        return NULL;
    }

    // Create process info to return as ProcessHandle
    ProcessInfo *processInfo = (ProcessInfo *)malloc(sizeof(ProcessInfo));
    if (processInfo == NULL) {
        fprintf(stderr, "Failed to allocate memory for process info\n");
        return NULL;
    }
    processInfo->thread_id = thread;
    return (ProcessHandle)processInfo;
}

void stopProcess(ProcessHandle processHandle) {
    if (processHandle == NULL) {
        fprintf(stderr, "Invalid process handle\n");
        return;
    }

    ProcessInfo *processInfo = (ProcessInfo *)processHandle;
    pthread_cancel(processInfo->thread_id);
    free(processInfo);
}

void delayProcess(int timeMs) {
    usleep(timeMs * 1000); 
}

void generateRandomNodeName(uint32_t *nodeName) {
    srand((unsigned int)getpid());
    *nodeName = (uint32_t)rand();
}

