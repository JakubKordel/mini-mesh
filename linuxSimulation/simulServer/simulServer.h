#ifndef SIMULSERVER_H
#define SIMULSERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <semaphore.h>
#include <stdint.h>

#define MAX_NODES 100
#define SERVER_PIPE_NAME "/tmp/server_pipe"
#define SERVER_PIPE_MUTEX_NAME "/server_pipe_mutex"
#define SERVER_PIPE_COUNTER_NAME "/server_pipe_counter"
#define NODE_PIPE_PREFIX "/tmp/node_pipe_"
#define NODE_PIPE_MUTEX_PREFIX "/node_pipe_mutex_"
#define NODE_PIPE_COUNTER_PREFIX "/node_pipe_counter_"
#define SIMUL_PRINTING_MUTEX_NAME "/linux_simulation_semaphore"


typedef struct NetworkNode {
    int nodeNum;
    int neighborsList[32];
    char pipeName[64];
    sem_t * pipeMutex;
    sem_t * pipeCounter;
} SimulNetworkNode;

typedef struct NetworkNodesList {
    SimulNetworkNode node[MAX_NODES];
} NetworkNodesList;

typedef struct {
    sem_t *serverPipeMutex;
    sem_t *serverPipeCountSemaphore;
    sem_t *printMutex;
} ServerResources;

extern int individualNodeNetworkNum;

void initServerResources(ServerResources *resources);
void deinitServerResources(ServerResources *resources);
void addNewNetworkNode(NetworkNodesList *list, int nodeNum, int neighborsList[32]);
void deleteNetworkNode(NetworkNodesList *list, int nodeNum);
void linuxSimulationToServerSend(uint8_t *msg, int len);
void linuxSimulationServerReceiveProcess(NetworkNodesList *nodesList);
void linuxSimulationServerPacketsHandler(NetworkNodesList *nodesList, uint8_t *msg, int len, int senderNum);
void linuxSimulationNodeReceiveProcess(void (*packetHandler)(uint8_t *, int));
void linuxSimulationToNodeSend(uint8_t *msg, int len, SimulNetworkNode *node);
void linuxSimulationPrint(const char *format, ...) ;

#endif /* SIMULSERVER_H */



