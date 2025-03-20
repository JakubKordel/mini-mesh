#ifndef MESHLIBRARYPROCESSES_H_
#define MESHLIBRARYPROCESSES_H_

#include <meshSystemStruct.h>
#include <stdint.h>

void meshClockInc(mesh_clock_handle * mch);
uint32_t getCurrentMeshTimestamp(mesh_clock_handle *mch);
uint32_t meshTimeDiff(uint32_t startVal, uint32_t endVal);
void transmitDataProcess(MeshSystem *params);
void inputPacketsHandlingProcess(MeshSystem *params);
void systemManagementProcess(MeshSystem * ms);
void packetReceivedCallback(MeshSystem * ms, void *data, int len);
void broadcastSelfInformationPacket(MeshSystem * ms);
void sendInfoRequestPacket(MeshSystem * ms, uint32_t destinationId);
int broadcastSubscriptions(DataReceivedCallbackTable *drct, MeshSystem * ms, uint32_t timestamp);

#endif /* MESHLIBRARYPROCESSES_H_ */