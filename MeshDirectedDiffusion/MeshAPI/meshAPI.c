#include "meshAPI.h"
#include <meshSystemStruct.h>
#include <packetsBuilder.h>
#include <neighborsTable.h>
#include <systemDependentHelpFunctions.h>
#include <PacketsBuffer.h>
#include <packetStructs.h>
#include <meshSystemConfig.h>

typedef struct mesh_network_handle {
	MeshSystem * meshSystem;
} mesh_network_handle_t;

int joinNetwork(mesh_network_handle_t **mnh, uint32_t meshId, uint8_t encrKey[16]) {
	
    uint32_t nodeName;
    generateRandomNodeName(&nodeName);

    *mnh = malloc(sizeof(mesh_network_handle_t));
    if (*mnh == NULL) {
        return -1; 
    }

    (*mnh)->meshSystem = initMeshSystem(meshId, nodeName, encrKey);
    return 0;
}

int subscribe(mesh_network_handle_t *mnh, InterestId * interestId, uint8_t maxDist, void (*callback)(void *, int)){

	addDataCallback(mnh->meshSystem->dataReceivedCallbackTable, *interestId, callback, maxDist);
	
	return 0;
}

int publish(mesh_network_handle_t *mnh, InterestId * interestId, void (*callback)( void * ), void * params ){
	
	addInterestCallback(mnh->meshSystem->interestCallbackTable, *interestId, callback, params);
	return 0;
}

int sendDataAlongGradient(mesh_network_handle_t * mnh, InterestId * interestId, void * data, uint16_t dataSize){
	
	if (dataSize > (MAX_TRANSMIT_PCKT_SIZE_BYTES - DATA_PACKET_HEADER_SIZE - 32)){
		// The data, along with the packet header and hash bytes, may not fit into a single transmitted packet; consider implementing a data division mechanism
		return -1;
	}
	
	uint32_t neighborsMask = 0U;

	int result = getGradient(mnh->meshSystem->gradientTable, *interestId, &neighborsMask);
	
	PacketHeader header;
	header.meshId = mnh->meshSystem->meshId;
	header.sourceId = mnh->meshSystem->nodeId;
	header.totalLength = DATA_PACKET_HEADER_SIZE + dataSize;
	header.packetDescr = (uint8_t)(DESCR_DATA_PACKET_BITS | DESCR_IS_MULTICAST_BIT);
	if (REQ_ACK_ON_DATA_PCKTS == 1) {
		header.packetDescr |= DESCR_REQ_ACK_BIT;
	}
	header.packetId = 0;
	
	DataPacket rest;
	rest.interestId = *interestId;
	rest.dataSize = dataSize;
	rest.data = data; 		
	
	if (result == 0 && neighborsMask != 0U) {

		void * packet = NULL; 
		uint16_t packetLen = header.totalLength;
		header.destinationId = neighborsMask;
		buildDataPacket(&packet, &packetLen, &header, &rest);
		putPacketToBuffer(mnh->meshSystem->packetsBuffer, neighborsMask, packet, packetLen);
		destroyPacket(&packet);
	} else {
		// TODO Handle the error case
	}
	
	return 0;
}

int leaveNetwork(mesh_network_handle_t **mnh) {
    if (*mnh) {
        deinitMeshSystem((*mnh)->meshSystem);
        free(*mnh); 
        *mnh = NULL; 
    }

    return 0;
}
