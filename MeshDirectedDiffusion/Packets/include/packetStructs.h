#ifndef PACKETSTRUCTS_H_
#define PACKETSTRUCTS_H_

#include <InterestId.h>

// Packet description bits
#define DESCR_PACKET_TYPE_MASK            0b11000000
#define DESCR_ACKNOWLEDGE_PACKET_BITS     0b00000000
#define DESCR_INTEREST_FLOOD_PACKET_BITS  0b01000000
#define DESCR_DATA_PACKET_BITS            0b10000000
#define DESCR_CONNECTION_PACKET_BITS      0b11000000
#define DESCR_IS_MULTICAST_BIT            0b00100000
#define DESCR_REQ_ACK_BIT                 0b00010000
#define DESCR_UNUSED_BITS_MASK            0b00001111

// Sizes of packet data without alignment (transmitted packet lengths), not the size of the structs
#define PACKET_HEADER_SIZE               	16
#define DATA_PACKET_HEADER_SIZE          	34
#define INTEREST_FLOOD_PACKET_HEADER_SIZE 	39
#define SELF_INFORMATION_PACKET_SIZE      	148
#define INFO_PACKET_REQ_SIZE			20

// Number of octets to shift from the start of the packet to access a particular packet field
#define SOURCE_ID_SHIFT                  	4
#define DESTINATION_ID_SHIFT             	8
#define PACKET_TOTAL_LENGTH_SHIFT        	12
#define PACKET_DESCR_SHIFT               	14
#define PACKET_ID_SHIFT                  	15
#define HOPS_COUNT_SHIFT                 	36
#define DATA_SIZE_SHIFT                  	32
#define MAX_PACKET_ID                    	254
#define PACKET_ID_NULL                   	255

// Connection packet description
#define CONN_PKT_SELF_INFO_PKT_CODE       0
#define CONN_PKT_INFO_PACKET_REQ_CODE	  1

// Interest description
#define INTEREST_DESCR_REINFORCEMENT_BIT  0b10000000
#define INTEREST_DESCR_UNUSED_BITS_MASK   0b01111111


// Structures for unloaded packet data (to ensure data is properly aligned)
typedef struct {
    uint32_t meshId;
    uint32_t sourceId;
    uint32_t destinationId;
    uint16_t totalLength;
    uint8_t packetDescr;
    uint8_t packetId;
} PacketHeader;

typedef struct {
    InterestId interestId;
    uint32_t uniqueSendersNumber;
    uint8_t hopsCount;
    uint8_t hopsLimit;
    uint8_t interestDescr;
} InterestFloodPacket;

typedef struct {
    InterestId interestId;
    uint16_t dataSize;
    uint8_t *data;
} DataPacket;

typedef struct {
    uint32_t connDescr;
    uint8_t *rest;
} ConnectionPacket;

typedef struct {
    uint32_t neighborName[32];
} NeighborsList;

typedef struct {
    uint8_t acknowledgedPacketId;
} AcknowledgePacket;

#endif /* PACKETSTRUCTS_H_ */
