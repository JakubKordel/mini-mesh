#include <packetsBuilder.h>
#include <stdlib.h>
#include <string.h>

int buildInterestFloodPacket(void **packet, uint16_t * packetLen, PacketHeader *header, InterestFloodPacket *interestFloodPacket) {
    uint16_t len = INTEREST_FLOOD_PACKET_HEADER_SIZE;
    
    void *pkt = malloc(len);
    if (pkt == NULL) {
        return -1;
    }
    
    *packetLen = len;
    
    uint8_t *ptr = (uint8_t *)pkt;
    
    memcpy(ptr, &header->meshId, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    
    memcpy(ptr, &header->sourceId, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    
    memcpy(ptr, &header->destinationId, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    
    memcpy(ptr, &header->totalLength, sizeof(uint16_t));
    ptr += sizeof(uint16_t);
    
    *ptr = header->packetDescr;
    ptr += sizeof(uint8_t);
    
    *ptr = header->packetId;
    ptr += sizeof(uint8_t);
    
    memcpy(ptr, &interestFloodPacket->interestId, sizeof(InterestId));
    ptr += sizeof(InterestId);
    
    memcpy(ptr, &interestFloodPacket->uniqueSendersNumber, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    
    *ptr = interestFloodPacket->hopsCount;
    ptr += sizeof(uint8_t);
    
    *ptr = interestFloodPacket->hopsLimit;
    ptr += sizeof(uint8_t);
    
    *ptr = interestFloodPacket->interestDescr;
    
    *packet = pkt;
    
    return 0;
}

int buildDataPacket(void **packet, uint16_t * packetLen, PacketHeader *header, DataPacket *dataPacket) {
    uint16_t len = DATA_PACKET_HEADER_SIZE + dataPacket->dataSize;
    
    void *pkt = malloc(len);
    if (pkt == NULL) {
        return -1; 
    }
    
    *packetLen = len;
    uint8_t *ptr = (uint8_t *)pkt;
    
    memcpy(ptr, &header->meshId, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    
    memcpy(ptr, &header->sourceId, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    
    memcpy(ptr, &header->destinationId, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    
    memcpy(ptr, &header->totalLength, sizeof(uint16_t));
    ptr += sizeof(uint16_t);
    
    *ptr = header->packetDescr;
    ptr += sizeof(uint8_t);
    
    *ptr = header->packetId;
    ptr += sizeof(uint8_t);
    
    memcpy(ptr, &dataPacket->interestId, sizeof(InterestId));
    ptr += sizeof(InterestId);
    
    memcpy(ptr, &dataPacket->dataSize, sizeof(uint16_t));
    ptr += sizeof(uint16_t);
    
    memcpy(ptr, dataPacket->data, dataPacket->dataSize);
    
    *packet = pkt;
    
    return 0;
}

int buildConnectionPacket(void ** packet, uint16_t * packetLen, PacketHeader *header, ConnectionPacket * connectionPacket) {
	
	uint16_t len = PACKET_HEADER_SIZE + 4 ;
	
	if (connectionPacket->connDescr == CONN_PKT_SELF_INFO_PKT_CODE){
		len = SELF_INFORMATION_PACKET_SIZE;
	}
    
    void *pkt = malloc(len);
    if (pkt == NULL) {
        return -1; 
    }
    
    *packetLen = len;
    
    uint8_t *ptr = (uint8_t *)pkt;
    
    memcpy(ptr, &header->meshId, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    
    memcpy(ptr, &header->sourceId, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    
    memcpy(ptr, &header->destinationId, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    
    memcpy(ptr, &header->totalLength, sizeof(uint16_t));
    ptr += sizeof(uint16_t);
    
    *ptr = header->packetDescr;
    ptr += sizeof(uint8_t);
    
    *ptr = header->packetId;
    ptr += sizeof(uint8_t);

	memcpy(ptr, &connectionPacket->connDescr, sizeof(uint32_t));
	ptr += sizeof(uint32_t);
	
	if (connectionPacket->connDescr == CONN_PKT_SELF_INFO_PKT_CODE){
		memcpy(ptr, connectionPacket->rest, 32 * sizeof(uint32_t));
		ptr += 32 * sizeof(uint32_t);
	}
	
	if (connectionPacket->connDescr == CONN_PKT_INFO_PACKET_REQ_CODE){
		//Nothing really
	}
    
    *packet = pkt;
    
    return 0;
}

int buildAcknowledgePacket(void ** packet, uint16_t * packetLen, PacketHeader *header){
	uint16_t len = PACKET_HEADER_SIZE;
    
    void *pkt = malloc(len);
    if (pkt == NULL) {
        return -1; 
    }
    
    *packetLen = len;
    
    uint8_t *ptr = (uint8_t *)pkt;
    
    memcpy(ptr, &header->meshId, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    
    memcpy(ptr, &header->sourceId, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    
    memcpy(ptr, &header->destinationId, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    
    memcpy(ptr, &header->totalLength, sizeof(uint16_t));
    ptr += sizeof(uint16_t);
    
    *ptr = header->packetDescr;
    ptr += sizeof(uint8_t);
    
    *ptr = header->packetId;
    ptr += sizeof(uint8_t);
	
	*packet = pkt;
	return 0;
}

int unloadPacketHeader(void *packet, PacketHeader *header) {
    uint8_t *ptr = (uint8_t *)packet;
    
    memcpy(&header->meshId, ptr, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    
    memcpy(&header->sourceId, ptr, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    
    memcpy(&header->destinationId, ptr, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    
    memcpy(&header->totalLength, ptr, sizeof(uint16_t));
    ptr += sizeof(uint16_t);
    
    header->packetDescr = *ptr;
    ptr += sizeof(uint8_t);
    
    header->packetId = *ptr;
    
    return 0;
}

int unloadInterestFloodPacket(void *packet, InterestFloodPacket *interestFloodPacket) {
    uint8_t *ptr = (uint8_t *)packet + PACKET_HEADER_SIZE;
    
    memcpy(&interestFloodPacket->interestId, ptr, sizeof(InterestId));
    ptr += sizeof(InterestId);
    
    memcpy(&interestFloodPacket->uniqueSendersNumber, ptr, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    
    interestFloodPacket->hopsCount = *ptr;
    ptr += sizeof(uint8_t);
    
    interestFloodPacket->hopsLimit = *ptr;
    
    return 0;
}

int unloadDataPacket(void *packet, DataPacket *dataPacket) {
    uint8_t *ptr = (uint8_t *)packet + PACKET_HEADER_SIZE;
    
    memcpy(&dataPacket->interestId, ptr, sizeof(InterestId));
    ptr += sizeof(InterestId);
    
    memcpy(&dataPacket->dataSize, ptr, sizeof(uint16_t));
    ptr += sizeof(uint16_t);
    
    dataPacket->data = ptr;
    
    return 0;
}

int unloadConnectionPacket(void * packet, ConnectionPacket * connectionPacket){
	uint8_t *ptr = (uint8_t *)packet + PACKET_HEADER_SIZE;
    
	memcpy(&connectionPacket->connDescr, ptr, sizeof(uint32_t));
	ptr += sizeof(uint32_t);

	connectionPacket->rest = ptr;

    return 0;
}

int destroyPacket(void **packet) {
    if (*packet != NULL) {
        free(*packet);
        *packet = NULL;
		return 0;
    }
	else return -1;
}
