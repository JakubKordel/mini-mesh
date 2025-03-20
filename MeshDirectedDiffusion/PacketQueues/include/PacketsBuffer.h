#ifndef PACKETS_BUFFER_H
#define PACKETS_BUFFER_H

#include <stdbool.h>
#include <stdint.h>
#include <universal_semaphore.h>

// Bit flags for the 'flags' field
#define PACKET_LOCKED_BIT     0b10000000
#define PACKET_SLEEP_BIT      0b01000000
#define PACKET_FOR_DROP_BIT   0b00100000
#define PACKET_INPUT_BIT      0b00010000
#define PACKET_OUTPUT_BIT     0b00001000

// Constants
#define ACTIVE_INPUT_PACKET      0b00010000
#define ACTIVE_OUTPUT_PACKET     0b00001000
#define SLEEPING_OUTPUT_PACKET   0b01001000

typedef struct {
    uint8_t flags;
    uint8_t transmissionsNum;
    uint8_t packetId;
    uint16_t packetPtr;
    uint16_t packetSize;
    uint32_t neighborsBitMask; // Assuming a maximum of 32 neighbors
    uint32_t lastTransmissionTimestamp;
} PacketDescr;

typedef struct {
    uint8_t maxPacketNum; // Maximum number of packets in the buffer, size of packetsDescrBuffer
    uint8_t numberOfPackets;
    uint8_t numberOfActiveInputPackets;
    uint8_t numberOfActiveOutputPackets;
    uint8_t descrHead; 
    uint8_t descrTail; 
    uint8_t nextPacketId;
    uint16_t head; 
    uint16_t tail; 
    uint16_t nextOutputPacket;
    uint16_t nextInputPacket;
    uint16_t freeSize; 
    uint16_t bufferSize; 
    uint16_t maxPacketSize; 
    PacketDescr *packetsDescrBuffer;
    uint8_t *buffer; 
    binary_semaphore_t *outputPacketsQueue;
    binary_semaphore_t *inputPacketsQueue;
    binary_semaphore_t *mutex;
} PacketsBuffer;

int initPacketsBuffer(PacketsBuffer **pb, uint16_t bufferSize, uint8_t maxPacketNum, uint16_t maxPacketSize);

int putPacketToBuffer(PacketsBuffer *pb, uint32_t neighborsBits, void *packet, uint16_t packet_len);

int lockNextOutputPacket(PacketsBuffer *pb, uint8_t *packetId, uint8_t **data1, uint16_t *data_len1, 
										uint8_t **data2, uint16_t *data_len2, uint32_t *neighborsBits);
										
int sleepOutputPacket(PacketsBuffer *pb, uint8_t packetId, uint32_t currentTimestamp);

int dropPacket(PacketsBuffer *pb, uint8_t packetId);

int wakeUpOutputPackets(PacketsBuffer *pb, uint32_t timeFromLastTransmissionDS, uint32_t currentTime);

int lockNextInputPacket(PacketsBuffer *pb, uint8_t *packetId, uint8_t **data1, uint16_t *data_len1, uint8_t **data2, uint16_t *data_len2);

int moveInputPacketToOutput(PacketsBuffer *pb, uint8_t packetId, uint32_t neighborsBits);

int receivedAcknowledgment(PacketsBuffer *pb, uint8_t packetId, uint32_t neighborBit);

int handleDisconnectionOfNeighbors(PacketsBuffer *pb, uint32_t updatedConnectedNeighborsBits);

int freePacketsBuffer(PacketsBuffer **pb);

void printPacketsBuffer(PacketsBuffer *packetsBuffer);

#endif // PACKETS_BUFFER_H
