#include "PacketsBuffer.h"
#include <meshLibraryProcesses.h>
#include <meshSystemConfig.h>

int performDroppingRoutine(PacketsBuffer *pb) {
    while (pb->numberOfPackets > 0 &&
           (pb->packetsDescrBuffer[pb->descrTail].flags & (PACKET_FOR_DROP_BIT | PACKET_LOCKED_BIT)) == PACKET_FOR_DROP_BIT) {
        pb->tail = (pb->tail + pb->packetsDescrBuffer[pb->descrTail].packetSize) % pb->bufferSize;
        pb->freeSize += pb->packetsDescrBuffer[pb->descrTail].packetSize;
        pb->descrTail = (pb->descrTail + 1) % pb->maxPacketNum;
        --pb->numberOfPackets;
    }
    return 0;
}

int initPacketsBuffer(PacketsBuffer **pb, uint16_t bufferSize, uint8_t maxPacketNum, uint16_t maxPacketSize) {
    *pb = malloc(sizeof(PacketsBuffer));
    (*pb)->bufferSize = bufferSize;
    (*pb)->maxPacketNum = maxPacketNum;
    (*pb)->maxPacketSize = maxPacketSize;
    (*pb)->freeSize = bufferSize;
    (*pb)->numberOfPackets = 0U;
    (*pb)->numberOfActiveOutputPackets = 0U;
    (*pb)->numberOfActiveInputPackets = 0U;
    (*pb)->head = 0U;
    (*pb)->tail = 0U;
    (*pb)->descrHead = 0U;
    (*pb)->descrTail = 0U;
    (*pb)->nextPacketId = 0U;
    (*pb)->packetsDescrBuffer = malloc(maxPacketNum * sizeof(PacketDescr));
    for (int i = 0U; i < maxPacketNum; ++i) {
        (*pb)->packetsDescrBuffer[i].flags |= PACKET_FOR_DROP_BIT;
    }
    (*pb)->buffer = malloc(bufferSize);
    (*pb)->nextOutputPacket = maxPacketNum;
    (*pb)->nextInputPacket = maxPacketNum;
    (*pb)->outputPacketsQueue = binary_semaphore_create(0);
    (*pb)->inputPacketsQueue = binary_semaphore_create(0);
    (*pb)->mutex = binary_semaphore_create(1);
    return 0;
}

int putPacketToBuffer(PacketsBuffer *pb, uint32_t neighborsBits, void *packet, uint16_t packet_len) {
    binary_semaphore_wait(pb->mutex);

    if (pb->freeSize < packet_len) {
        binary_semaphore_post(pb->mutex);
        return -1;
    }
    if (pb->numberOfPackets == pb->maxPacketNum) {
        binary_semaphore_post(pb->mutex);
        return -2;
    }

    uint16_t spaceToEnd = pb->bufferSize - pb->head;
    if (packet_len > spaceToEnd) {
        memcpy(&pb->buffer[pb->head], packet, spaceToEnd);
        memcpy(&pb->buffer[0], packet + spaceToEnd, packet_len - spaceToEnd);
    } else {
        memcpy(&pb->buffer[pb->head], packet, packet_len);
    }
    pb->packetsDescrBuffer[pb->descrHead].packetPtr = pb->head;
    pb->packetsDescrBuffer[pb->descrHead].packetSize = packet_len;
    pb->packetsDescrBuffer[pb->descrHead].neighborsBitMask = neighborsBits;
    pb->packetsDescrBuffer[pb->descrHead].transmissionsNum = 0;
    pb->packetsDescrBuffer[pb->descrHead].packetId = pb->nextPacketId;

    pb->nextPacketId = (pb->nextPacketId + 1) % ((256 / pb->maxPacketNum) * pb->maxPacketNum);
    pb->head = (pb->head + packet_len) % pb->bufferSize;

    if (neighborsBits == 0x00000000) {
        ++pb->numberOfActiveInputPackets;
        pb->packetsDescrBuffer[pb->descrHead].flags = ACTIVE_INPUT_PACKET;
        if (pb->numberOfActiveInputPackets == 1) {
            pb->nextInputPacket = pb->descrHead;
            binary_semaphore_post(pb->inputPacketsQueue);
        }
    } else {
        ++pb->numberOfActiveOutputPackets;
        pb->packetsDescrBuffer[pb->descrHead].flags = ACTIVE_OUTPUT_PACKET;
        if (pb->numberOfActiveOutputPackets == 1) {
            pb->nextOutputPacket = pb->descrHead;
            binary_semaphore_post(pb->outputPacketsQueue);
        }
    }

    pb->descrHead = (pb->descrHead + 1) % pb->maxPacketNum;
    pb->freeSize -= packet_len;
    ++pb->numberOfPackets;

    binary_semaphore_post(pb->mutex);

    return 0;
}

int lockNextOutputPacket(PacketsBuffer *pb, uint8_t *packetId, uint8_t **data1, uint16_t *data_len1, uint8_t **data2, uint16_t *data_len2, uint32_t *neighborsBits) {
	
	if (binary_semaphore_wait_timeoutMS(pb->outputPacketsQueue, 500) != 1){
		return -2; 
	}

    binary_semaphore_wait(pb->mutex);

    if (pb->numberOfActiveOutputPackets == 0) {
        *data1 = NULL;
        *data_len1 = 0;
        *data2 = NULL;
        *data_len2 = 0;
        *packetId = 0;
        *neighborsBits = 0;
        binary_semaphore_post(pb->mutex);
        return -1;
    }

    pb->packetsDescrBuffer[pb->nextOutputPacket].flags |= PACKET_LOCKED_BIT;

    *packetId = pb->packetsDescrBuffer[pb->nextOutputPacket].packetId;
    *neighborsBits = pb->packetsDescrBuffer[pb->nextOutputPacket].neighborsBitMask;

    uint16_t spaceToEnd = pb->bufferSize - pb->packetsDescrBuffer[pb->nextOutputPacket].packetPtr;

    if (pb->packetsDescrBuffer[pb->nextOutputPacket].packetSize > spaceToEnd) {
        // packet is split into two parts
        *data1 = &pb->buffer[pb->packetsDescrBuffer[pb->nextOutputPacket].packetPtr];
        *data_len1 = spaceToEnd;
        *data2 = &pb->buffer[0];
        *data_len2 = pb->packetsDescrBuffer[pb->nextOutputPacket].packetSize - spaceToEnd;
    } else {
        // packet fits in one part in the buffer
        *data1 = &pb->buffer[pb->packetsDescrBuffer[pb->nextOutputPacket].packetPtr];
        *data_len1 = pb->packetsDescrBuffer[pb->nextOutputPacket].packetSize;
        *data2 = NULL;
        *data_len2 = 0;
    }

    if (--pb->numberOfActiveOutputPackets > 0) {
        do {
            pb->nextOutputPacket = (pb->nextOutputPacket + 1) % pb->maxPacketNum;
            if (pb->nextOutputPacket == pb->descrHead) {
                pb->nextOutputPacket = pb->descrTail;
            }
        } while (pb->packetsDescrBuffer[pb->nextOutputPacket].flags != ACTIVE_OUTPUT_PACKET);
        binary_semaphore_post(pb->outputPacketsQueue);
    } else {
        pb->nextOutputPacket = pb->maxPacketNum;
    }

    binary_semaphore_post(pb->mutex);

    return 0;
}

int sleepOutputPacket(PacketsBuffer *pb, uint8_t packetId, uint32_t currentTimestamp){
	binary_semaphore_wait(pb->mutex);
	if ( (pb->packetsDescrBuffer[packetId % pb->maxPacketNum].flags & PACKET_FOR_DROP_BIT) != PACKET_FOR_DROP_BIT && packetId == pb->packetsDescrBuffer[packetId % pb->maxPacketNum].packetId ){
		pb->packetsDescrBuffer[packetId % pb->maxPacketNum].flags &= ~PACKET_LOCKED_BIT;
		++pb->packetsDescrBuffer[packetId % pb->maxPacketNum].transmissionsNum;
		pb->packetsDescrBuffer[packetId % pb->maxPacketNum].lastTransmissionTimestamp = currentTimestamp;
		if (pb->packetsDescrBuffer[packetId % pb->maxPacketNum].transmissionsNum < MAX_RETRANSMISSIONS_NUM){
			pb->packetsDescrBuffer[packetId % pb->maxPacketNum].flags |= PACKET_SLEEP_BIT;
		} else {
			pb->packetsDescrBuffer[packetId % pb->maxPacketNum].flags |= PACKET_FOR_DROP_BIT;
			performDroppingRoutine(pb);
		}
	} else if (packetId == pb->packetsDescrBuffer[packetId % pb->maxPacketNum].packetId){
		pb->packetsDescrBuffer[packetId % pb->maxPacketNum].flags &= ~PACKET_LOCKED_BIT;
		performDroppingRoutine(pb);
	}
	
	binary_semaphore_post(pb->mutex);
	return 0;
}

int dropPacket(PacketsBuffer *pb, uint8_t packetId){
	binary_semaphore_wait(pb->mutex);
	
	if ( packetId == pb->packetsDescrBuffer[packetId % pb->maxPacketNum].packetId){
		pb->packetsDescrBuffer[packetId % pb->maxPacketNum].flags |= PACKET_FOR_DROP_BIT;
		pb->packetsDescrBuffer[packetId % pb->maxPacketNum].flags &= ~PACKET_LOCKED_BIT;
		performDroppingRoutine(pb);
	} 	
	
	binary_semaphore_post(pb->mutex);
	
	return 0;
}

int wakeUpOutputPackets(PacketsBuffer *pb, uint32_t timeFromLastTransmissionDS, uint32_t currentTime){
	binary_semaphore_wait(pb->mutex);
	
	if (pb->numberOfPackets > 0){
		uint16_t packetPtr = pb->descrTail;
		bool flag = false;
		do {
			if (!flag && pb->packetsDescrBuffer[packetPtr].flags == ACTIVE_OUTPUT_PACKET ){
				flag = true;
				pb->nextOutputPacket = packetPtr;
			} else if (pb->packetsDescrBuffer[packetPtr].flags == SLEEPING_OUTPUT_PACKET 
					&& meshTimeDiff(pb->packetsDescrBuffer[packetPtr].lastTransmissionTimestamp, currentTime) > timeFromLastTransmissionDS){
				pb->packetsDescrBuffer[packetPtr].flags &= ~PACKET_SLEEP_BIT;
				++pb->numberOfActiveOutputPackets;
				if (!flag){
					flag = true;
					if (pb->nextOutputPacket == pb->maxPacketNum) {
						binary_semaphore_post(pb->outputPacketsQueue);
					}
					pb->nextOutputPacket = packetPtr;
				}
			}
			packetPtr = (packetPtr + 1) % pb->maxPacketNum;
		} while (packetPtr != pb->descrHead);
	}
	binary_semaphore_post(pb->mutex);
	
	return 0;
}

int lockNextInputPacket(PacketsBuffer *pb, uint8_t * packetId, uint8_t **data1, uint16_t *data_len1, uint8_t **data2, uint16_t *data_len2){
	if (binary_semaphore_wait_timeoutMS(pb->inputPacketsQueue, 500) != 1){
		return -2; 
	}
	
	binary_semaphore_wait(pb->mutex);
	
	if (pb->numberOfActiveInputPackets == 0){
			*data1 = NULL;
			*data_len1 = 0;
			*data2 = NULL;
			*data_len2 = 0;
			*packetId = 0;
			binary_semaphore_post(pb->mutex);	
			return -1;
	}
	
	pb->packetsDescrBuffer[pb->nextInputPacket].flags |= PACKET_LOCKED_BIT; 
	
	*packetId = pb->packetsDescrBuffer[pb->nextInputPacket].packetId;
	
	uint16_t spaceToEnd = pb->bufferSize - pb->packetsDescrBuffer[pb->nextInputPacket].packetPtr;
	
    if (pb->packetsDescrBuffer[pb->nextInputPacket].packetSize > spaceToEnd) {
        // packet is split into two parts
        *data1 = &pb->buffer[pb->packetsDescrBuffer[pb->nextInputPacket].packetPtr] ;
		*data_len1 = spaceToEnd;
		*data2 = &pb->buffer[0];
		*data_len2 = pb->packetsDescrBuffer[pb->nextInputPacket].packetSize - spaceToEnd;		
    } else {
        // packet fits in one part in the buffer
        *data1 = &pb->buffer[pb->packetsDescrBuffer[pb->nextInputPacket].packetPtr] ;
		*data_len1 = pb->packetsDescrBuffer[pb->nextInputPacket].packetSize;
		*data2 = NULL;
		*data_len2 = 0;
    }
	
	if (--pb->numberOfActiveInputPackets > 0){
		do {
			pb->nextInputPacket = (pb->nextInputPacket + 1) % pb->maxPacketNum;
			if (pb->nextInputPacket == pb->descrHead){
				pb->nextInputPacket = pb->descrTail;
			}
		}
		while (pb->packetsDescrBuffer[pb->nextInputPacket].flags != ACTIVE_INPUT_PACKET );
		binary_semaphore_post(pb->inputPacketsQueue);
		
	} else {
		pb->nextInputPacket = pb->maxPacketNum;
	}
	
	binary_semaphore_post(pb->mutex);

	return 0;
}

int moveInputPacketToOutput(PacketsBuffer *pb, uint8_t packetId, uint32_t neighborsBits){
	binary_semaphore_wait(pb->mutex);
	
	if ( (pb->packetsDescrBuffer[packetId % pb->maxPacketNum].flags & PACKET_FOR_DROP_BIT) != PACKET_FOR_DROP_BIT 
			&& packetId == pb->packetsDescrBuffer[packetId % pb->maxPacketNum].packetId){
				
		++pb->numberOfActiveOutputPackets;
		pb->packetsDescrBuffer[packetId % pb->maxPacketNum].flags = ACTIVE_OUTPUT_PACKET;
		pb->packetsDescrBuffer[packetId % pb->maxPacketNum].neighborsBitMask = neighborsBits;
		if (pb->numberOfActiveOutputPackets == 1){
			pb->nextOutputPacket = packetId % pb->maxPacketNum;
			binary_semaphore_post(pb->outputPacketsQueue);
		}
	} 
	
	binary_semaphore_post(pb->mutex);
	
	return 0;
}

int receivedAcknowledgment(PacketsBuffer *pb, uint8_t packetId, uint32_t neighborBit){
	binary_semaphore_wait(pb->mutex);
	
	if ( (pb->packetsDescrBuffer[packetId % pb->maxPacketNum].flags & PACKET_FOR_DROP_BIT) != PACKET_FOR_DROP_BIT 
			&& packetId == pb->packetsDescrBuffer[packetId % pb->maxPacketNum].packetId 
			&& (pb->packetsDescrBuffer[packetId % pb->maxPacketNum].flags & PACKET_OUTPUT_BIT) == PACKET_OUTPUT_BIT){
		pb->packetsDescrBuffer[packetId % pb->maxPacketNum].neighborsBitMask &= ~neighborBit; 
		
		if (pb->packetsDescrBuffer[packetId % pb->maxPacketNum].neighborsBitMask == 0x00000000){
			if (pb->packetsDescrBuffer[packetId % pb->maxPacketNum].flags == ACTIVE_OUTPUT_PACKET){
				--pb->numberOfActiveOutputPackets;
				if (pb->numberOfActiveOutputPackets > 0 && pb->nextOutputPacket == packetId % pb->maxPacketNum ){
					do {
						pb->nextOutputPacket = (pb->nextOutputPacket + 1) % pb->maxPacketNum;
						if (pb->nextOutputPacket == pb->descrHead){
							pb->nextOutputPacket = pb->descrTail;
						}
					} while (pb->packetsDescrBuffer[pb->nextOutputPacket].flags != ACTIVE_OUTPUT_PACKET );
				} else if (pb->numberOfActiveOutputPackets == 0){
					pb->nextOutputPacket = pb->maxPacketNum;
				}
			}
			pb->packetsDescrBuffer[packetId % pb->maxPacketNum].flags |= PACKET_FOR_DROP_BIT;
			performDroppingRoutine(pb);
		}
	} 	
	
	binary_semaphore_post(pb->mutex);
	
	return 0;
}

int handleDisconnectionOfNeighbors(PacketsBuffer *pb, uint32_t updatedConnectedNeighborsBits){
	binary_semaphore_wait(pb->mutex);
	
	if (pb->numberOfPackets > 0){
		uint16_t packetPtr = pb->descrTail;
		do {
			if ((pb->packetsDescrBuffer[packetPtr].flags & PACKET_FOR_DROP_BIT) != PACKET_FOR_DROP_BIT && (pb->packetsDescrBuffer[packetPtr].flags & PACKET_OUTPUT_BIT) == PACKET_OUTPUT_BIT ){
				pb->packetsDescrBuffer[packetPtr].neighborsBitMask &= updatedConnectedNeighborsBits;
				if (pb->packetsDescrBuffer[packetPtr].neighborsBitMask == 0){
					if (pb->packetsDescrBuffer[packetPtr].flags == ACTIVE_OUTPUT_PACKET){
							--pb->numberOfActiveOutputPackets;
					}
					pb->packetsDescrBuffer[packetPtr].flags |= PACKET_FOR_DROP_BIT;
				}
			}
			packetPtr = (packetPtr + 1) % pb->maxPacketNum;
		} while (packetPtr != pb->descrHead);
		if (pb->numberOfActiveOutputPackets > 0 && (pb->packetsDescrBuffer[pb->nextOutputPacket].flags & PACKET_FOR_DROP_BIT) == PACKET_FOR_DROP_BIT){
			do {
				pb->nextOutputPacket = (pb->nextOutputPacket + 1) % pb->maxPacketNum;
				if (pb->nextOutputPacket == pb->descrHead){
					pb->nextOutputPacket = pb->descrTail;
				}
			}
			while (pb->packetsDescrBuffer[pb->nextOutputPacket].flags != ACTIVE_OUTPUT_PACKET );
		} else if (pb->numberOfActiveOutputPackets == 0){
			pb->nextOutputPacket = pb->maxPacketNum;
		}
		performDroppingRoutine(pb);
	}
	
	binary_semaphore_post(pb->mutex);
	
	return 0;
}

int freePacketsBuffer(PacketsBuffer **pb) {
    if (!pb || !(*pb)) {
        return -1; 
    }

    binary_semaphore_destroy((*pb)->outputPacketsQueue);
    binary_semaphore_destroy((*pb)->inputPacketsQueue);
    binary_semaphore_destroy((*pb)->mutex);

    free((*pb)->packetsDescrBuffer);
    free((*pb)->buffer);

    free(*pb);

    *pb = NULL;
	
	return 0;
}

#include <stdio.h>

void printPacketsBuffer(PacketsBuffer *packetsBuffer) {
    binary_semaphore_wait(packetsBuffer->mutex);

    printf("Packets Buffer Information:\n");
    printf("Max Packet Num: %u\n", packetsBuffer->maxPacketNum);
    printf("Number of Packets: %u\n", packetsBuffer->numberOfPackets);
    printf("Number of Active Input Packets: %u\n", packetsBuffer->numberOfActiveInputPackets);
    printf("Number of Active Output Packets: %u\n", packetsBuffer->numberOfActiveOutputPackets);
    printf("Descr Head: %u\n", packetsBuffer->descrHead);
    printf("Descr Tail: %u\n", packetsBuffer->descrTail);
    printf("Next Packet ID: %u\n", packetsBuffer->nextPacketId);
    printf("Head: %u\n", packetsBuffer->head);
    printf("Tail: %u\n", packetsBuffer->tail);
    printf("Next Output Packet: %u\n", packetsBuffer->nextOutputPacket);
    printf("Next Input Packet: %u\n", packetsBuffer->nextInputPacket);
    printf("Free Size: %u\n", packetsBuffer->freeSize);
    printf("Buffer Size: %u\n", packetsBuffer->bufferSize);
    printf("Max Packet Size: %u\n", packetsBuffer->maxPacketSize);

    printf("\n");

	printf("Packet Descriptor Buffer (In Use):\n");
    printf("%-10s%-20s%-20s%-20s%-20s%-20s\n", "Packet ID", "Transmissions", "Packet Size", "Neighbors Bitmask", "Timestamp", "Flags");

    for (uint8_t i = packetsBuffer->descrTail; i != packetsBuffer->descrHead; i = (i + 1) % packetsBuffer->maxPacketNum) {
        PacketDescr *descr = &packetsBuffer->packetsDescrBuffer[i];

        printf("%-10u%-20u%-20u%-20u%-20u%-20u\n",
               descr->packetId,
               descr->transmissionsNum,
               descr->packetSize,
               descr->neighborsBitMask,
               descr->lastTransmissionTimestamp,
               descr->flags);
    }

    binary_semaphore_post(packetsBuffer->mutex);
}