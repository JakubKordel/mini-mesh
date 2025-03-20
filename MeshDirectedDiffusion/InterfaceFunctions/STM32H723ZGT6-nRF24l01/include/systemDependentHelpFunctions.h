/* 
 * SystemDependentHelpFunctions.h
 * 
 * This file contains declarations of system-dependent helper functions
 * that need to be implemented based on the target environment or platform.
 * Please provide implementations for these functions.
 */

#ifndef SYSTEMDEPENDENTHELPFUNCTIONS_H_
#define SYSTEMDEPENDENTHELPFUNCTIONS_H_

#include <stdint.h>

typedef void* ProcessHandle;

typedef void (*ProcessFunction)(void *);

ProcessHandle startProcess(ProcessFunction processFunction, const char *processName, uint32_t stackSize, void *parameters, uint32_t priority);

void stopProcess(ProcessHandle processHandle);

void delayProcess(int timeMs);

void generateRandomNodeName(uint32_t *nodeName);

#endif //SYSTEMDEPENDENTHELPFUNCTIONS_H_
