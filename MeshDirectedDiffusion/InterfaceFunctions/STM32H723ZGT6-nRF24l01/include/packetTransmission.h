/**
 * @file packetTransmission.h
 * @brief Functions for packet transmission (environment-dependent implementation).
 */

#ifndef PACKETTRANSMISSION_H_
#define PACKETTRANSMISSION_H_

#include <meshSystemStruct.h>

/**
 * 
 */
int initTransmissionFunctionality(MeshSystem *meshSystem);

/**
 * 
 */
int transmit_data(MeshSystem *meshSystem, uint8_t *packetPtr, int length);

/**
 * 
 */
int deinitTransmissionFunctionality(MeshSystem *meshSystem);

#endif /* PACKETTRANSMISSION_H_ */
