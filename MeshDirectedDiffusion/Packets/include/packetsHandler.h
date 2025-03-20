#ifndef PACKETSHANDLER_H_
#define PACKETSHANDLER_H_

#include <meshSystemStruct.h>
#include <stdint.h>

int handlePacket(MeshSystem * ms, void *packet, int len, uint32_t * neighborsBits);

int getPacketSize(MeshSystem * ms, void * packet, int * len);

bool amIPacketDestination(MeshSystem * ms, void * packet, int len);

#endif /* PACKETSHANDLER_H_*/
