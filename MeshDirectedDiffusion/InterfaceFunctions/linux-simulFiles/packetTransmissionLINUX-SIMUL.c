/**
 * @file packetTransmission.c
 * @brief Packet transmission functions implementation for simulation environment
 *
 * 
 */

#include "packetTransmission.h"

#include <meshLibraryProcesses.h>
#include <meshSystemConfig.h>
#include <packetEncryption.h>
#include <systemDependentHelpFunctions.h>

#include <simulServer.h>

static MeshSystem * mSystem = NULL;
ProcessHandle recvPrc;

void receivedDataThroughSimulation(uint8_t *data, int data_len) {

    packetReceivedCallback(mSystem, data, data_len);
}

int initTransmissionFunctionality(MeshSystem *meshSystem) {
    mSystem = meshSystem;

    initEncryption(&mSystem->encryptionHandler);
    
    recvPrc = startProcess((ProcessFunction)linuxSimulationNodeReceiveProcess, "linuxSimulationNodeReceiveProcess", 1024, (void *)receivedDataThroughSimulation, 10);

    return 0;
}

int transmit_data(MeshSystem *meshSystem, uint8_t *packetPtr, int length) {
    linuxSimulationToServerSend(packetPtr, length);
}

int deinitTransmissionFunctionality(MeshSystem *meshSystem) {
    deinitEncryption(&meshSystem->encryptionHandler);
    stopProcess(recvPrc);

    return 0;
}
