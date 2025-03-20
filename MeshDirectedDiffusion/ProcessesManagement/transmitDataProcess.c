#include <meshLibraryProcesses.h>
#include <packetTransmission.h>
#include <PacketsBuffer.h>
#include <packetStructs.h>
#include <systemDependentHelpFunctions.h>
#include <meshSystemConfig.h>

void transmitDataProcess(MeshSystem * ms) {

    PacketHeader packetHeader;
    packetHeader.meshId = ms->meshId;
    packetHeader.sourceId = ms->nodeId;

    while (1) {
        uint8_t transmitedPacket[MAX_TRANSMIT_PCKT_SIZE_BYTES];
        uint8_t * packetHandle;
        uint8_t packetId;
        uint8_t * part1 = NULL;
        uint8_t * part2 = NULL;
        uint16_t part1len = 0;
        uint16_t part2len = 0;
        uint32_t neighborsMask = 0;
        int newLen = MAX_TRANSMIT_PCKT_SIZE_BYTES;

        while(lockNextOutputPacket(ms->packetsBuffer, &packetId, &part1, &part1len, &part2, &part2len, &neighborsMask) != 0 ){
			if (ms->shouldTerminate) {
				counting_semaphore_post(ms->terminateSemaphore); 
				stopProcess(NULL);
			}
		}

        if (part2 != NULL) {
            memcpy(transmitedPacket, part1, part1len);
            memcpy(transmitedPacket + part1len, part2, part2len);
            packetHandle = transmitedPacket;
        } else {
            packetHandle = part1;
        }

        memcpy(&packetHeader.packetDescr, packetHandle + PACKET_DESCR_SHIFT, sizeof(uint8_t));
        memcpy(&packetHeader.destinationId, packetHandle + DESTINATION_ID_SHIFT, sizeof(uint32_t));
        if ((packetHeader.packetDescr & DESCR_IS_MULTICAST_BIT) == DESCR_IS_MULTICAST_BIT) {
            memcpy(packetHandle + DESTINATION_ID_SHIFT, &neighborsMask, sizeof(uint32_t));
        }

        if ((packetHeader.packetDescr & DESCR_PACKET_TYPE_MASK) != DESCR_ACKNOWLEDGE_PACKET_BITS) {
            memcpy(packetHandle + PACKET_ID_SHIFT, &packetId, sizeof(uint8_t));
        }
        addPacketHash(&ms->encryptionHandler, packetHandle, part1len + part2len, (void*)transmitedPacket, &newLen);
		if (ENCRYPT_PACKETS != 0){
			encryptPacket(&ms->encryptionHandler, (void*)transmitedPacket + PACKET_HEADER_SIZE, newLen - PACKET_HEADER_SIZE);
		}
        int tr_ret = transmit_data(ms, transmitedPacket, newLen);
        if (tr_ret != 0) {
            // TODO: Handle error
        } else {
            if ((packetHeader.packetDescr & DESCR_REQ_ACK_BIT) != DESCR_REQ_ACK_BIT) {
                dropPacket(ms->packetsBuffer, packetId);
            } else {
                sleepOutputPacket(ms->packetsBuffer, packetId, getCurrentMeshTimestamp(ms->clock));
            }
        }

		if (ms->shouldTerminate) {
			counting_semaphore_post(ms->terminateSemaphore); 
			stopProcess(NULL);
		}
		
		delayProcess(TRANSMISSIONS_BASE_DELAY_MS);
    }
}
