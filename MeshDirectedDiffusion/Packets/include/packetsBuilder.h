#ifndef PACKETSBUILDER_H_
#define PACKETSBUILDER_H_

#include <InterestId.h>
#include <stdint.h>
#include "packetStructs.h"

int buildInterestFloodPacket(void **packet, uint16_t * packetLen, PacketHeader *header, InterestFloodPacket *interestFloodPacket);
int buildDataPacket(void **packet, uint16_t * packetLen, PacketHeader *header, DataPacket *dataPacket);
int buildConnectionPacket(void ** packet, uint16_t * packetLen, PacketHeader *header, ConnectionPacket * connectionPacket);
int buildAcknowledgePacket(void ** packet, uint16_t * packetLen, PacketHeader *header);

int unloadPacketHeader(void *packet, PacketHeader *header);
int unloadInterestFloodPacket(void *packet, InterestFloodPacket *interestFloodPacket);
int unloadDataPacket(void *packet, DataPacket *dataPacket);
int unloadConnectionPacket(void * packet, ConnectionPacket * connectionPacket);

int destroyPacket(void **packet);

#endif /* PACKETSBUILDER_H_ */