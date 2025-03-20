#include <InterestId.h>
#include <meshAPI.h>
#include <unistd.h> 
#include "../simulServer/simulServer.h"

#include <stdlib.h> 
#include <time.h>   

#define MESH_ID 1

#define INTEREST_ID_PART_1 0x00000000
#define INTEREST_ID_PART_2 0x00000000
#define INTEREST_ID_PART_3 0x00000000
#define INTEREST_ID_PART_4 0x00000001

void receivedInterestHandler(void * params){

}

void senderNode(){
    InterestId interestId;
    makeInterestId(&interestId, INTEREST_ID_PART_1, INTEREST_ID_PART_2, INTEREST_ID_PART_3, INTEREST_ID_PART_4);
    
    uint8_t encrKey[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

    mesh_network_handle_t *mnh = NULL;
    joinNetwork(&mnh, MESH_ID, encrKey);
    
    publish(mnh, &interestId, receivedInterestHandler, (void *) mnh);
    
    char data[23] = "Some random hello data";
    
    srand(individualNodeNetworkNum);
    
    while (1){
        data[20] = rand() % 26 + 'a'; 
        data[21] = rand() % 26 + 'A'; 
        data[22] = rand() % 10 + '0'; 
        
        linuxSimulationPrint("Sending data: %s (Size: %d)\n", (char*)data, 23);
        sendDataAlongGradient(mnh, &interestId, data, 23);
        sleep(1);
    }
}
