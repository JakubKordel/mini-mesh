#include "universal_semaphore.h"
#include <stdint.h>

#include <systemDependentHelpFunctions.h>
#include <meshLibraryProcesses.h>
#include <meshSystemConfig.h>

#include <neighborsTable.h>
#include <dataReceivedCallbackTable.h>

void systemManagementProcess(MeshSystem * ms){
		int i = 0;
		
		while(1){	
			if (i % 1 == 0){ //with these settings mesh clock increase always, mesh clock increase once every 100ms
				meshClockInc(ms->clock); //mesh clock isnt required to be that accurate, its more about system ticks, here system tick is around 100ms
			}
			
			uint32_t currentTimestamp = getCurrentMeshTimestamp(ms->clock);
			
			if (i % 100 == 0){

				disconnectSilentNeighbors(ms->neighborsTable, currentTimestamp, NEIGHBOR_INACTIVITY_TIMEOUT_SEC * 7); 
			
				handleDisconnectionOfNeighbors(ms->packetsBuffer, ms->neighborsTable->connectedMask);
				
				//refreshGradientTable(ms->gradientTable, ms->neighborsTable->connectedMask, currentTimestamp, GRADIENT_INACTIVITY_TIMEOUT_SEC * 10);
			}
			
			wakeUpOutputPackets(ms->packetsBuffer, ACK_TIMEOUT_DS, currentTimestamp);
			
			if (i % 100 == 0 ){
				broadcastSelfInformationPacket(ms);
			}
			
			if (i % 10 == 0 ){
				broadcastSubscriptions(ms->dataReceivedCallbackTable, ms, currentTimestamp);
			}
			
			++i;
			
			if (ms->shouldTerminate) {
				counting_semaphore_post(ms->terminateSemaphore); 
				stopProcess(NULL);
			}
			delayProcess(100);
		}
}
