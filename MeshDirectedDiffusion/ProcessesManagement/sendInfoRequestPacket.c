#include <meshLibraryProcesses.h>
#include <packetsBuilder.h>
#include <neighborsTable.h>
#include <packetStructs.h>
#include <PacketsBuffer.h>

#define BROADCAST_ADRESS 0xFFFFFFFF


void sendInfoRequestPacket(MeshSystem * ms, uint32_t destinationId){

        void * packet = NULL;
        uint16_t packetLen;

        PacketHeader header;
        header.meshId = ms->meshId;
        header.sourceId = ms->nodeId;
        header.destinationId = destinationId;
        header.totalLength = INFO_PACKET_REQ_SIZE;
        header.packetDescr = DESCR_CONNECTION_PACKET_BITS;
        header.packetId = 0;

        ConnectionPacket connectionPacket;
        connectionPacket.connDescr = CONN_PKT_INFO_PACKET_REQ_CODE;
        connectionPacket.rest = NULL;

        buildConnectionPacket(&packet, &packetLen, &header, &connectionPacket);
        putPacketToBuffer(ms->packetsBuffer, BROADCAST_ADRESS, packet, packetLen);

        destroyPacket(&packet);
}