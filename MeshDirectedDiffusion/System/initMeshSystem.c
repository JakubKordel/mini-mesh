#include <meshSystemStruct.h>
#include <meshLibraryProcesses.h>
#include <universal_semaphore.h>
#include <gradientTable.h>
#include <interestCallbackTable.h>
#include <neighborsTable.h>
#include <packetTransmission.h>
#include <systemDependentHelpFunctions.h>
#include <PacketsBuffer.h>
#include <inputPacketsHashTable.h>
#include <dataHashTable.h>
#include <meshSystemConfig.h>

MeshSystem * initMeshSystem(uint32_t meshId, uint32_t nodeId, uint8_t encrKey[16]){

	MeshSystem * meshSystem = (MeshSystem *)malloc(sizeof(MeshSystem));
	
	memset(meshSystem, 0, sizeof(MeshSystem));
	
	meshSystem->maxNeighbors = MAX_NEIGHBORS_NUM;
	
	meshSystem->meshId = meshId;
    meshSystem->nodeId = nodeId;
	
	initGradientTable(&meshSystem->gradientTable);
	
	initInterestCallbackTable(&meshSystem->interestCallbackTable);
	
	initNeighborTable(&meshSystem->neighborsTable);
	
	initDataReceivedCallbackTable(&meshSystem->dataReceivedCallbackTable);
	
	meshSystem->inputPacketsHashTable = hash_table_init();
	
	meshSystem->dataHashTable = dataHashTableInit();
	
	initPacketsBuffer(&meshSystem->packetsBuffer, PCKTS_BUFF_SPACE_BYTES, MAX_PCKTS_IN_BUFFER_NUM, MAX_TRANSMIT_PCKT_SIZE_BYTES - 32);
	
	memcpy(meshSystem->encryptionHandler.key, encrKey, 16);
	meshSystem->encryptionHandler.key[16] = '\0';
	
	meshSystem->clock = (mesh_clock_handle *)malloc(sizeof(mesh_clock_handle));
	meshSystem->clock->sem = binary_semaphore_create(1);
	meshSystem->clock->value = (uint32_t *)malloc(sizeof(uint32_t));
	*meshSystem->clock->value = 0;

	initTransmissionFunctionality(meshSystem);
	
	meshSystem->shouldTerminate = false;
	meshSystem->terminateSemaphore = counting_semaphore_create(0);
	
	meshSystem->systemManagementProcessHandle = startProcess((ProcessFunction)systemManagementProcess, "SystemManagement", 2048, (void *)meshSystem, SYSTEM_MANAGEMENT_PROCESS_PRIORITY);
	meshSystem->transmitDataProcessHandle = startProcess((ProcessFunction)transmitDataProcess, "TransmitData", 4096, (void *)meshSystem, TRANSMIT_DATA_PROCESS_PRIORITY); 
	meshSystem->inputPacketsHandlingProcessHandle = startProcess((ProcessFunction)inputPacketsHandlingProcess, "InputPackets", 4096, (void *)meshSystem, INPUT_PCKT_HNDL_PROCESS_PRIORITY); 
	
	return meshSystem;
}

void deinitMeshSystem(MeshSystem * ms){

	ms->shouldTerminate = true;

	counting_semaphore_wait(ms->terminateSemaphore);
	counting_semaphore_wait(ms->terminateSemaphore);
	counting_semaphore_wait(ms->terminateSemaphore);
	counting_semaphore_destroy(ms->terminateSemaphore);
	
	deinitTransmissionFunctionality(ms);
	
	binary_semaphore_destroy(ms->clock->sem);
    free(ms->clock->value);
    free(ms->clock);

    freePacketsBuffer(&ms->packetsBuffer);
	
	hash_table_deinit(ms->inputPacketsHashTable);
	
	dataHashTableDeinit(ms->dataHashTable);

    freeGradientTable(ms->gradientTable);
    freeInterestCallbackTable(ms->interestCallbackTable);
    freeNeighborTable(ms->neighborsTable);
	freeDataCallbackTable(ms->dataReceivedCallbackTable);
	
    free(ms);
}
