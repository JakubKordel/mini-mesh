#include <meshLibraryProcesses.h>
#include <packetsBuilder.h>
#include <neighborsTable.h>
#include <packetStructs.h>
#include <PacketsBuffer.h>

#define BROADCAST_ADRESS 0xFFFFFFFF

void broadcastSelfInformationPacket(MeshSystem * ms){
	
        NeighborsList list;

        getNeighborsNamesList(ms->neighborsTable, list.neighborName);

        void * packet = NULL;
        uint16_t packetLen;

        PacketHeader header;
        header.meshId = ms->meshId;
        header.sourceId = ms->nodeId;
        header.destinationId = BROADCAST_ADRESS;
        header.totalLength = SELF_INFORMATION_PACKET_SIZE;
        header.packetDescr = DESCR_CONNECTION_PACKET_BITS;
        header.packetId = 0;

        ConnectionPacket connectionPacket;
        connectionPacket.connDescr = CONN_PKT_SELF_INFO_PKT_CODE;
        connectionPacket.rest = (uint8_t*)list.neighborName;

        buildConnectionPacket(&packet, &packetLen, &header, &connectionPacket);
        putPacketToBuffer(ms->packetsBuffer, BROADCAST_ADRESS, packet, packetLen);

        destroyPacket(&packet);
}

