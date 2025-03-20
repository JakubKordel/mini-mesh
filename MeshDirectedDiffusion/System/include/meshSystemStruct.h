#ifndef MESHSYSTEMSTRUCT_H_
#define MESHSYSTEMSTRUCT_H_

#include <universal_semaphore.h>
#include <neighborsTable.h>
#include <interestCallbackTable.h>
#include <gradientTable.h>
#include <dataReceivedCallbackTable.h>
#include <packetEncryption.h>
#include <PacketsBuffer.h>
#include <inputPacketsHashTable.h>
#include <systemDependentHelpFunctions.h>
#include <dataHashTable.h>
#include <stdint.h>

typedef struct {
    binary_semaphore_t *sem;
    uint32_t *value;
} mesh_clock_handle;

typedef struct {
	//Basics configurations 
	uint32_t meshId;
    uint32_t nodeId;
    int maxNeighbors;
	
	//System tables
	NeighborsTable *neighborsTable;
	GradientTable *gradientTable;
	InterestCallbackTable *interestCallbackTable;
	DataReceivedCallbackTable *dataReceivedCallbackTable;
	HashTable * inputPacketsHashTable;
	DataHashTable * dataHashTable;
	
	PacketsBuffer * packetsBuffer;
	
	mesh_clock_handle *clock;
	
	EncryptionHandler encryptionHandler;
	
	ProcessHandle systemManagementProcessHandle;
	ProcessHandle transmitDataProcessHandle;
	ProcessHandle inputPacketsHandlingProcessHandle;
	counting_semaphore_t * terminateSemaphore;
	bool shouldTerminate;  
} MeshSystem;

MeshSystem *initMeshSystem(uint32_t meshId, uint32_t nodeId, uint8_t encrKey[16]);
void deinitMeshSystem(MeshSystem * ms);

#endif /* MESHSYSTEMSTRUCT_H_ */