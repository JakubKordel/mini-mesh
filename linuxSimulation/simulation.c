#include "simulServer/simulServer.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h> 

#define NODES_NUM 15

pid_t childProcesses[NODES_NUM];
NetworkNodesList nodesList;
ServerResources serverResources;

void sigintHandler(int sig) {
    for (int i = 0; i < NODES_NUM; ++i) {
        if (childProcesses[i] > 0) {
            kill(childProcesses[i], SIGTERM);
        }
        deleteNetworkNode(&nodesList, i);
    }
    deinitServerResources(&serverResources);
    exit(EXIT_SUCCESS); 
}

int main() {
    signal(SIGINT, sigintHandler);

    initServerResources(&serverResources);

    char taskForNodeName[NODES_NUM][64]; 

    int neighborsList[32];
          
    memset(neighborsList, -1, sizeof(neighborsList));
    neighborsList[0] = 1;
    addNewNetworkNode(&nodesList, 0, neighborsList);
    strcpy(taskForNodeName[0], "relayingNode");

    memset(neighborsList, -1, sizeof(neighborsList));
    neighborsList[0] = 0;neighborsList[1] = 2;
    addNewNetworkNode(&nodesList, 1, neighborsList);
    strcpy(taskForNodeName[1], "senderNode");

    memset(neighborsList, -1, sizeof(neighborsList));
    neighborsList[0] = 1;neighborsList[1] = 4;
    addNewNetworkNode(&nodesList, 2, neighborsList);
    strcpy(taskForNodeName[2], "relayingNode");

    memset(neighborsList, -1, sizeof(neighborsList));
    neighborsList[0] = 4;
    addNewNetworkNode(&nodesList, 3, neighborsList);
    strcpy(taskForNodeName[3], "relayingNode");

    memset(neighborsList, -1, sizeof(neighborsList));
    neighborsList[0] = 2;neighborsList[1] = 3;neighborsList[2] = 5;
    addNewNetworkNode(&nodesList, 4, neighborsList);
    strcpy(taskForNodeName[4], "relayingNode");

    memset(neighborsList, -1, sizeof(neighborsList));
    neighborsList[0] = 4;neighborsList[1] = 6;neighborsList[2] = 9;
    addNewNetworkNode(&nodesList, 5, neighborsList);
    strcpy(taskForNodeName[5], "relayingNode");

    memset(neighborsList, -1, sizeof(neighborsList));
    neighborsList[0] = 4;neighborsList[1] = 5;neighborsList[2] = 7;neighborsList[3] = 9;neighborsList[4] = 10;
    addNewNetworkNode(&nodesList, 6, neighborsList);
    strcpy(taskForNodeName[6], "receiverNode");

    memset(neighborsList, -1, sizeof(neighborsList));
    neighborsList[0] = 6;neighborsList[1] = 9;
    addNewNetworkNode(&nodesList, 7, neighborsList);
    strcpy(taskForNodeName[7], "relayingNode");

    memset(neighborsList, -1, sizeof(neighborsList));
    neighborsList[0] = 9;
    addNewNetworkNode(&nodesList, 8, neighborsList);
    strcpy(taskForNodeName[8], "relayingNode");

    memset(neighborsList, -1, sizeof(neighborsList));
    neighborsList[0] = 5;neighborsList[1] = 6;neighborsList[2] = 7;neighborsList[3] = 8;
    addNewNetworkNode(&nodesList, 9, neighborsList);
    strcpy(taskForNodeName[9], "senderNode");

    memset(neighborsList, -1, sizeof(neighborsList));
    neighborsList[0] = 6;neighborsList[1] = 11;
    addNewNetworkNode(&nodesList, 10, neighborsList);
    strcpy(taskForNodeName[10], "relayingNode");

    memset(neighborsList, -1, sizeof(neighborsList));
    neighborsList[0] = 10;neighborsList[1] = 12;neighborsList[2] = 13;
    addNewNetworkNode(&nodesList, 11, neighborsList);
    strcpy(taskForNodeName[11], "relayingNode");

    memset(neighborsList, -1, sizeof(neighborsList));
    neighborsList[0] = 11;neighborsList[1] = 14;
    addNewNetworkNode(&nodesList, 12, neighborsList);
    strcpy(taskForNodeName[12], "relayingNode");

    memset(neighborsList, -1, sizeof(neighborsList));
    neighborsList[0] = 11;
    addNewNetworkNode(&nodesList, 13, neighborsList);
    strcpy(taskForNodeName[13], "relayingNode");

    memset(neighborsList, -1, sizeof(neighborsList));
    neighborsList[0] = 12;
    addNewNetworkNode(&nodesList, 14, neighborsList);
    strcpy(taskForNodeName[14], "relayingNode");

    // Start network nodes
    for (int i = 0; i < NODES_NUM; ++i) {
        pid_t pid = fork();
        if (pid == -1) {
            return -1;
        } else if (pid == 0) {
            char nodeNumArg[16];
            snprintf(nodeNumArg, sizeof(nodeNumArg), "%d", i);
            char *args[] = {"./NodeStarter", nodeNumArg, taskForNodeName[i], NULL};
            execvp("./NodeStarter", args);
            perror("execvp");
            return -1;
        } else {
            childProcesses[i] = pid;
        }
    }

    linuxSimulationServerReceiveProcess(&nodesList);

    return 0;
}
