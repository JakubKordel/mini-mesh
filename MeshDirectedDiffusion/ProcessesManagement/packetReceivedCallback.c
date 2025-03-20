#include "meshLibraryProcesses.h"
#include <PacketsBuffer.h>
#include <packetStructs.h>
#include <packetsBuilder.h>
#include <inputPacketsHashTable.h>
#include <meshLibraryProcesses.h>
#include <packetEncryption.h>
#include <packetsHandler.h>
#include <meshSystemConfig.h>

#define MINIMUM_RECEIVED_DATA_SIZE 48
#define HASH_LEN 32

void packetReceivedCallback(MeshSystem *ms, void *data, int len) {
	
    if (len >= MINIMUM_RECEIVED_DATA_SIZE && len <= MAX_TRANSMIT_PCKT_SIZE_BYTES && amIPacketDestination(ms, data, PACKET_HEADER_SIZE)) {

		if (ENCRYPT_PACKETS != 0){
			decryptPacket(&ms->encryptionHandler, data + PACKET_HEADER_SIZE, len - PACKET_HEADER_SIZE);
		}
		
        if (validatePacketHash(&ms->encryptionHandler, data, len)) {
	
            int packetSize = len - HASH_LEN;
            getPacketSize(ms, data, &packetSize);
            if (packetSize <= MAX_TRANSMIT_PCKT_SIZE_BYTES - HASH_LEN ) {

				if (*((uint8_t*)(data + PACKET_DESCR_SHIFT)) == DESCR_ACKNOWLEDGE_PACKET_BITS){
					
					uint32_t sourceId;
					memcpy(&sourceId, data + SOURCE_ID_SHIFT, sizeof(uint32_t));
			
					uint32_t nBit;

					int err = getNeighborMask(ms->neighborsTable, sourceId, &nBit);

					if (err == 0){
						receivedAcknowledgment(ms->packetsBuffer, *(uint8_t*)(data + PACKET_ID_SHIFT), nBit);
					}
					
				} else if (*((uint8_t *)(data + PACKET_DESCR_SHIFT)) & DESCR_REQ_ACK_BIT) {
                    void *packet;
                    uint16_t packetLen;
                    PacketHeader header;
                    header.meshId = ms->meshId;
                    header.sourceId = ms->nodeId;
                    memcpy(&header.destinationId, (uint32_t *)(data + SOURCE_ID_SHIFT), sizeof(uint32_t));
                    header.totalLength = DATA_PACKET_HEADER_SIZE;
                    header.packetDescr = DESCR_ACKNOWLEDGE_PACKET_BITS;
                    memcpy(&header.packetId, (uint8_t *)(data + PACKET_ID_SHIFT), sizeof(uint8_t));
                    buildAcknowledgePacket(&packet, &packetLen, &header);
                    putPacketToBuffer(ms->packetsBuffer, 0xFFFFFFFF, packet, packetLen);
                    destroyPacket(&packet);

                    if (!hash_table_exists(ms->inputPacketsHashTable, (uint8_t *)(data + packetSize))) {
                        hash_table_insert(ms->inputPacketsHashTable, (uint8_t *)(data + packetSize));

                        int ret = putPacketToBuffer(ms->packetsBuffer, 0, data, packetSize);

						if (ret != 0) {
							// Handle error
						}
                    } else {
                        // Handle the case where the packet is a duplicate
                    }
                } else {
                    int ret = putPacketToBuffer(ms->packetsBuffer, 0x00000000, data, packetSize);
                    if (ret != 0) {
                        // Handle error
                    }
                }
            } else {
                // Handle invalid packet
            }
        } else {
            // Handle invalid hash
        }
    } else {
        // Handle invalid data length
    }
}
