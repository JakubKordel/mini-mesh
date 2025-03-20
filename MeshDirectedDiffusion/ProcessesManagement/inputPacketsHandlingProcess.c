#include <PacketsBuffer.h>
#include <meshLibraryProcesses.h>
#include <packetsHandler.h>
#include <systemDependentHelpFunctions.h>

void inputPacketsHandlingProcess(MeshSystem *ms) {

    while (1) {

        uint8_t data[MAX_TRANSMIT_PCKT_SIZE_BYTES - 32];
		uint8_t * packetHandle = NULL;
		uint16_t data_len = 0;
		uint8_t * data1 = NULL;
		uint8_t * data2 = NULL;
		uint16_t data_len1;
		uint16_t data_len2;
		uint8_t packetId;
		uint32_t neighborsBits = 0U;
		
		while (lockNextInputPacket(ms->packetsBuffer, &packetId, &data1, &data_len1, &data2, &data_len2) != 0 ){
			if (ms->shouldTerminate) {
				counting_semaphore_post(ms->terminateSemaphore); 
				stopProcess(NULL);
			}
		}
		
		data_len = data_len1 + data_len2;
		if (data2 == NULL){
			packetHandle = data1;
		} else {
			memcpy(data, data1, data_len1);
			memcpy(data + data_len1, data2, data_len2);
			packetHandle = data;
		}
		
        if (data_len > 0) {
			handlePacket(ms, packetHandle, data_len, &neighborsBits);
			
			if (neighborsBits == 0U){
					dropPacket(ms->packetsBuffer, packetId);		
			} else {
				if (data2 != NULL){
					memcpy(data1, data, data_len1);
					memcpy(data2, data + data_len1, data_len2);
				}
				
				moveInputPacketToOutput(ms->packetsBuffer, packetId, neighborsBits); 
			}	
        }
		if (ms->shouldTerminate) {
			counting_semaphore_post(ms->terminateSemaphore); 
			stopProcess(NULL);
		}
    }
}
