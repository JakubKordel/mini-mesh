#include "packetsHandler.h" 
#include <meshSystemStruct.h>
#include <InterestId.h>
#include <gradientTable.h>
#include <meshLibraryProcesses.h>
#include <packetsBuilder.h>
#include <packetStructs.h>
#include <PacketsBuffer.h>
#include <dataHashTable.h>

#include "mbedtls/sha256.h"

int handlePacket(MeshSystem * ms, void *packet, int len, uint32_t * neighborsBits) {
	
	PacketHeader packetHeader;
	InterestFloodPacket interestFloodPacket;
	DataPacket dataPacket;
	ConnectionPacket connectionPacket;
    unloadPacketHeader(packet, &packetHeader);
	
	int ret;
	
	*neighborsBits = 0U;
	
    switch (packetHeader.packetDescr & DESCR_PACKET_TYPE_MASK) {
        case DESCR_INTEREST_FLOOD_PACKET_BITS:
        		
			unloadInterestFloodPacket(packet, &interestFloodPacket);
			 
			interestFloodPacket.hopsCount++;
				
			uint32_t neighborBit;
			
			if (getNeighborMask(ms->neighborsTable, packetHeader.sourceId, &neighborBit) == 0){
				gradientHandleInterestArrival(ms->gradientTable, interestFloodPacket.interestId, interestFloodPacket.uniqueSendersNumber, neighborBit, getCurrentMeshTimestamp(ms->clock), neighborsBits);
			}
				
			if (*neighborsBits != 0U && interestFloodPacket.hopsCount < interestFloodPacket.hopsLimit){
				memcpy(packet + SOURCE_ID_SHIFT, &ms->nodeId, sizeof(uint32_t));
				memcpy(packet + HOPS_COUNT_SHIFT, &interestFloodPacket.hopsCount, sizeof(uint8_t));
			} else {
				*neighborsBits = 0U;
			}
			
			void (*interestCallback)(void *params);
			void *params = NULL;
				
			ret = getInterestCallback(ms->interestCallbackTable, interestFloodPacket.interestId, &interestCallback, &params);

			if (ret == 0) {
				if (interestCallback != NULL) {
					(*interestCallback)(params); 
				}			
			}
       
			break;
            
        case DESCR_DATA_PACKET_BITS:
        		
			unloadDataPacket(packet, &dataPacket);

			uint8_t dataHash[32]; 
			mbedtls_sha256(dataPacket.data, dataPacket.dataSize, (unsigned char*)dataHash, 0);
			
			if (!dataHashTableExists(ms->dataHashTable, dataHash)){
				dataHashTableInsert(ms->dataHashTable, dataHash);
				
				uint32_t nBit;
				if (getNeighborMask(ms->neighborsTable, packetHeader.sourceId, &nBit) == 0){
					gradientHandleNewDataArrival(ms->gradientTable, dataPacket.interestId, nBit);
				}
				
				int result = getGradient(ms->gradientTable, dataPacket.interestId, neighborsBits);
				if (result == 0 && *neighborsBits != 0U) {
					memcpy(packet + SOURCE_ID_SHIFT, &ms->nodeId, sizeof(uint32_t));
				} 
				
				void (*dataCallback)(void*, int);
				ret = getDataCallback(ms->dataReceivedCallbackTable, dataPacket.interestId, &dataCallback);
				if (ret == 0) {
					if (dataCallback != NULL) {
						(*dataCallback)(dataPacket.data, dataPacket.dataSize);
					}
				}
			}
			break;
			
        case DESCR_CONNECTION_PACKET_BITS:
		
			unloadConnectionPacket(packet, &connectionPacket);
			if (connectionPacket.connDescr == CONN_PKT_SELF_INFO_PKT_CODE){
				bool gotSelfMulticastBitNumber = false;
				for (int i = 0 ; i < 32; ++i){
					if (memcmp(connectionPacket.rest + i * sizeof(uint32_t), &ms->nodeId, sizeof(uint32_t)) == 0 ){
						setSelfMulticastBitNumber(ms->neighborsTable, packetHeader.sourceId, i);
						gotSelfMulticastBitNumber = true;
						break;
					}
				}
				if (!gotSelfMulticastBitNumber) {
					sendInfoRequestPacket(ms, packetHeader.sourceId);
				}
			} else if (connectionPacket.connDescr == CONN_PKT_INFO_PACKET_REQ_CODE){
				broadcastSelfInformationPacket(ms);
			}
			 
			break;
        default:
   
            break;
    }
 
    return 0;
}

bool amIPacketDestination(MeshSystem * ms, void * packet, int len){
	if (len < 16){
		return false;
	}
	PacketHeader packetHeader;
    unloadPacketHeader(packet, &packetHeader);
	if (packetHeader.meshId != ms->meshId || packetHeader.sourceId == ms->nodeId){
		return false;
	}
	
	// WARNING ADDING neighbor that might not even have proper encryption key... 
	int ret = neighborHeardHandle(ms->neighborsTable, packetHeader.sourceId, getCurrentMeshTimestamp(ms->clock));
	if (ret == -1){
		//TODO action if max neighbors num reached ... or just will modify neighborSelfInfoReceived to replace someone
	} else if (ret == 1){
		broadcastSelfInformationPacket(ms);
	}
	
	if ((packetHeader.packetDescr & DESCR_IS_MULTICAST_BIT) != DESCR_IS_MULTICAST_BIT){
		return packetHeader.destinationId == 0xFFFFFFFF || packetHeader.destinationId == ms->nodeId;
	} else {
		uint8_t multicastBitNumber; 
		int err = getSelfMulticastBitNumber(ms->neighborsTable, packetHeader.sourceId, &multicastBitNumber);
		uint32_t bit = 1U << multicastBitNumber;
		return err == 0 && bit == (packetHeader.destinationId & bit);
	}
}

int getPacketSize(MeshSystem * ms, void * packet, int * len){
	if (*len < 16){
		return -1;
	}
	uint16_t packetSize;
	memcpy(&packetSize, packet + PACKET_TOTAL_LENGTH_SHIFT, sizeof(uint16_t));
	if (*len < packetSize){
		return -2;
	}
	*len = packetSize;
	return 0;
}
