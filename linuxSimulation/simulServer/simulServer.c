#include "simulServer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>

int individualNodeNetworkNum;

ssize_t read_struct(int fd, void *buf, size_t count) {
    ssize_t bytes_read = 0;
    ssize_t total_bytes_read = 0;

    while (total_bytes_read < count) {
        bytes_read = read(fd, buf + total_bytes_read, count - total_bytes_read);
        if (bytes_read == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        } else if (bytes_read == 0) {
            break;
        }
        total_bytes_read += bytes_read;
    }

    return total_bytes_read;
}

void initServerResources(ServerResources *resources) {
    sem_unlink(SERVER_PIPE_MUTEX_NAME);

    sem_unlink(SERVER_PIPE_COUNTER_NAME);

    unlink(SERVER_PIPE_NAME);

    sem_unlink(SIMUL_PRINTING_MUTEX_NAME);

    resources->serverPipeMutex = sem_open(SERVER_PIPE_MUTEX_NAME, O_CREAT , 0666, 1);
    if (resources->serverPipeMutex == SEM_FAILED) {
        perror("sem_open for serverPipeMutex");
        exit(EXIT_FAILURE);
    }

    resources->serverPipeCountSemaphore = sem_open(SERVER_PIPE_COUNTER_NAME, O_CREAT , 0666, 0);
    if (resources->serverPipeCountSemaphore == SEM_FAILED) {
        perror("sem_open for serverPipeCountSemaphore");
        exit(EXIT_FAILURE);
    }

    if (mkfifo(SERVER_PIPE_NAME, 0666) == -1) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }

    resources->printMutex = sem_open(SIMUL_PRINTING_MUTEX_NAME, O_CREAT , 0666, 1);
    if (resources->printMutex == SEM_FAILED) {
        perror("sem_open for printMutex");
        exit(EXIT_FAILURE);
    }
    
}

void deinitServerResources(ServerResources *resources) {
    if (sem_close(resources->serverPipeMutex) == -1) {
        perror("sem_close for serverPipeMutex");
        exit(EXIT_FAILURE);
    }

    if (sem_close(resources->serverPipeCountSemaphore) == -1) {
        perror("sem_close for serverPipeCountSemaphore");
        exit(EXIT_FAILURE);
    }

    if (sem_close(resources->printMutex) == -1) {
        perror("sem_close for printMutex");
        exit(EXIT_FAILURE);
    }

    if (unlink(SERVER_PIPE_NAME) == -1) {
        perror("unlink for server pipe");
        exit(EXIT_FAILURE);
    }

    if (sem_unlink(SERVER_PIPE_MUTEX_NAME) == -1) {
        perror("sem_unlink for serverPipeMutex");
        exit(EXIT_FAILURE);
    }

    if (sem_unlink(SERVER_PIPE_COUNTER_NAME) == -1) {
        perror("sem_unlink for serverPipeCountSemaphore");
        exit(EXIT_FAILURE);
    }

}


void addNewNetworkNode(NetworkNodesList *list, int nodeNum, int neighborsList[32]) {

    list->node[nodeNum].nodeNum = nodeNum;
    memcpy(list->node[nodeNum].neighborsList, neighborsList, sizeof(int) * 32);


    char pipeName[64];
    sprintf(pipeName, "%s%d", NODE_PIPE_PREFIX, nodeNum);
    strcpy(list->node[nodeNum].pipeName, pipeName);


    if (unlink(pipeName) == -1 && errno != ENOENT) {
        perror("unlink");
        exit(EXIT_FAILURE);
    }

    if (mkfifo(pipeName, 0666) == -1) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }

    char mutexName[64];
    char counterName[64];
    sprintf(mutexName, "%s%d", NODE_PIPE_MUTEX_PREFIX, nodeNum);
    sprintf(counterName, "%s%d", NODE_PIPE_COUNTER_PREFIX, nodeNum);

    sem_unlink(mutexName);

    sem_unlink(counterName);

    sem_t *mutex_sem = sem_open(mutexName, O_CREAT, 0666, 1);
    if (mutex_sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    list->node[nodeNum].pipeMutex = mutex_sem;

    sem_t *counter_sem = sem_open(counterName, O_CREAT, 0666, 0);
    if (counter_sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    list->node[nodeNum].pipeCounter = counter_sem;

}


void deleteNetworkNode(NetworkNodesList *list, int nodeNum) {
    if (unlink(list->node[nodeNum].pipeName) == -1 && errno != ENOENT) {
        perror("unlink");
        exit(EXIT_FAILURE);
    }

    if (sem_close(list->node[nodeNum].pipeMutex) == -1) {
        perror("sem_close");
        exit(EXIT_FAILURE);
    }

    if (sem_close(list->node[nodeNum].pipeCounter) == -1) {
        perror("sem_close");
        exit(EXIT_FAILURE);
    }

    char mutexName[64];
    sprintf(mutexName, "%s%d", NODE_PIPE_MUTEX_PREFIX, nodeNum);
    sem_unlink(mutexName);

    char counterName[64];
    sprintf(counterName, "%s%d", NODE_PIPE_COUNTER_PREFIX, nodeNum);
    sem_unlink(counterName);

    sem_destroy(list->node[nodeNum].pipeMutex);
    sem_destroy(list->node[nodeNum].pipeCounter);

    free(list->node[nodeNum].pipeMutex);
    free(list->node[nodeNum].pipeCounter);
}


void linuxSimulationToServerSend(uint8_t *msg, int len) {
    sem_t *serverPipeMutex;   
    sem_t *serverPipeCounter; 

    serverPipeMutex = sem_open(SERVER_PIPE_MUTEX_NAME, 0);
    if (serverPipeMutex == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    serverPipeCounter = sem_open(SERVER_PIPE_COUNTER_NAME, 0);
    if (serverPipeCounter == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    sem_wait(serverPipeMutex);
    
    int serverPipe = open(SERVER_PIPE_NAME, O_WRONLY);
    if (serverPipe < 0) {
        perror("Error opening server pipe");
        sem_post(serverPipeMutex);
        return;
    }
    
    write(serverPipe, &individualNodeNetworkNum, sizeof(int));
    write(serverPipe, &len, sizeof(int));
    write(serverPipe, msg, len);
    
    close(serverPipe);
    
    sem_post(serverPipeCounter);
    sem_post(serverPipeMutex);
}

void linuxSimulationServerReceiveProcess(NetworkNodesList *nodesList) {
    sem_t *serverPipeMutex;
    sem_t *serverPipeCountSemaphore;

    serverPipeMutex = sem_open(SERVER_PIPE_MUTEX_NAME, 0);
    if (serverPipeMutex == SEM_FAILED) {
        perror("sem_open for serverPipeMutex");
        exit(EXIT_FAILURE);
    }

    serverPipeCountSemaphore = sem_open(SERVER_PIPE_COUNTER_NAME, 0);
    if (serverPipeCountSemaphore == SEM_FAILED) {
        perror("sem_open for serverPipeCountSemaphore");
        exit(EXIT_FAILURE);
    }
    

    int serverPipe = open(SERVER_PIPE_NAME, O_RDONLY);

    while (1) {
        sem_wait(serverPipeCountSemaphore);
        sem_wait(serverPipeMutex);
        
        if (serverPipe < 0) {
            perror("Error opening server pipe");
            sem_post(serverPipeMutex);
            continue;
        }
        
        int senderNum;
        int len;
        read_struct(serverPipe, &senderNum, sizeof(int));
        read_struct(serverPipe, &len, sizeof(int));
        uint8_t *msg = malloc(len);
        read_struct(serverPipe, msg, len);
        
        sem_post(serverPipeMutex);
        
        linuxSimulationServerPacketsHandler(nodesList, msg, len, senderNum);
        
        free(msg);
    }
    close(serverPipe);
}

void linuxSimulationServerPacketsHandler(NetworkNodesList *nodesList, uint8_t *msg, int len, int senderNum) {
    SimulNetworkNode *node = NULL;
    
    if (nodesList->node[senderNum].nodeNum == senderNum) {
        node = &nodesList->node[senderNum];
    }
    
    if (node != NULL) {
        for (int i = 0; i < 32; i++) {
            if (node->neighborsList[i] >= 0 && node->neighborsList[i] < MAX_NODES && nodesList->node[node->neighborsList[i]].nodeNum == node->neighborsList[i]) {
                linuxSimulationToNodeSend(msg, len, &(nodesList->node[node->neighborsList[i]]));
            }
            //*** eventually modify idea so transmission information is stored somewhere else and delay accepting transmission so implementation of collisions is possible 
        }
    }
}

void linuxSimulationNodeReceiveProcess(void (*packetHandler)(uint8_t *, int)) {
    char nodePipeName[64];  
    sem_t *nodePipeMutex;  
    sem_t *nodePipeCounter; 

    sprintf(nodePipeName, "%s%d", NODE_PIPE_PREFIX, individualNodeNetworkNum);

    char mutexName[64];
    sprintf(mutexName, "%s%d", NODE_PIPE_MUTEX_PREFIX, individualNodeNetworkNum);
    nodePipeMutex = sem_open(mutexName, 0);
    if (nodePipeMutex == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    char counterName[64];
    sprintf(counterName, "%s%d", NODE_PIPE_COUNTER_PREFIX, individualNodeNetworkNum);
    nodePipeCounter = sem_open(counterName, 0);
    if (nodePipeCounter == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    
    int nodePipe = open(nodePipeName, O_RDONLY);

    while (1) {
        sem_wait(nodePipeCounter);
        sem_wait(nodePipeMutex);
        
        int msgSize;
        read_struct(nodePipe, &msgSize, sizeof(int));
        
        uint8_t *msg = malloc(msgSize);
        
        read_struct(nodePipe, msg, msgSize);
        
        sem_post(nodePipeMutex);
        
        packetHandler(msg, msgSize);
            
        free(msg);
    }
    close(nodePipe);
}

void linuxSimulationToNodeSend(uint8_t *msg, int len, SimulNetworkNode *node){

    int nodePipe = open(node->pipeName, O_WRONLY);
    if (nodePipe < 0) {
        perror("Error opening node pipe");
        sem_post(node->pipeMutex);
        return;
    }
    
    sem_wait(node->pipeMutex);
    
    write(nodePipe, &len, sizeof(int));
    
    write(nodePipe, msg, len);
    
    sem_post(node->pipeCounter);
    sem_post(node->pipeMutex);
    close(nodePipe);
}

void linuxSimulationPrint(const char *format, ...) {

    sem_t *printMutex = sem_open(SIMUL_PRINTING_MUTEX_NAME, 0); 
    if (printMutex == SEM_FAILED) {
       perror("Failed to open semaphore");
        return;
    }

    sem_wait(printMutex);
    
    printf("NODE %d: ", individualNodeNetworkNum);

    va_list args;
    va_start(args, format);
    
    vprintf(format, args);

    va_end(args);
    

    sem_post(printMutex);

    sem_close(printMutex);
}
