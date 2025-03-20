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
    // Unlink existing server pipe mutex
    sem_unlink(SERVER_PIPE_MUTEX_NAME);

    // Unlink existing server pipe count semaphore
    sem_unlink(SERVER_PIPE_COUNTER_NAME);

    // Unlink existing server pipe (named pipe)
    unlink(SERVER_PIPE_NAME);

    // Unlink existing print mutex semaphore
    sem_unlink(SIMUL_PRINTING_MUTEX_NAME);

    // Create server pipe mutex
    resources->serverPipeMutex = sem_open(SERVER_PIPE_MUTEX_NAME, O_CREAT , 0666, 1);
    if (resources->serverPipeMutex == SEM_FAILED) {
        perror("sem_open for serverPipeMutex");
        exit(EXIT_FAILURE);
    }

    // Create server pipe count semaphore
    resources->serverPipeCountSemaphore = sem_open(SERVER_PIPE_COUNTER_NAME, O_CREAT , 0666, 0);
    if (resources->serverPipeCountSemaphore == SEM_FAILED) {
        perror("sem_open for serverPipeCountSemaphore");
        exit(EXIT_FAILURE);
    }

    // Create server pipe
    if (mkfifo(SERVER_PIPE_NAME, 0666) == -1) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }

    // Create named semaphore (linux_simulation_semaphore)
    resources->printMutex = sem_open(SIMUL_PRINTING_MUTEX_NAME, O_CREAT , 0666, 1);
    if (resources->printMutex == SEM_FAILED) {
        perror("sem_open for printMutex");
        exit(EXIT_FAILURE);
    }
    
    // Optionally, initialize other resources needed for the server
}

void deinitServerResources(ServerResources *resources) {
    // Close server pipe mutex
    if (sem_close(resources->serverPipeMutex) == -1) {
        perror("sem_close for serverPipeMutex");
        exit(EXIT_FAILURE);
    }

    // Close server pipe count semaphore
    if (sem_close(resources->serverPipeCountSemaphore) == -1) {
        perror("sem_close for serverPipeCountSemaphore");
        exit(EXIT_FAILURE);
    }

    // Close print mutex semaphore
    if (sem_close(resources->printMutex) == -1) {
        perror("sem_close for printMutex");
        exit(EXIT_FAILURE);
    }

    // Unlink server pipe
    if (unlink(SERVER_PIPE_NAME) == -1) {
        perror("unlink for server pipe");
        exit(EXIT_FAILURE);
    }

    // Unlink server pipe mutex
    if (sem_unlink(SERVER_PIPE_MUTEX_NAME) == -1) {
        perror("sem_unlink for serverPipeMutex");
        exit(EXIT_FAILURE);
    }

    // Unlink server pipe count semaphore
    if (sem_unlink(SERVER_PIPE_COUNTER_NAME) == -1) {
        perror("sem_unlink for serverPipeCountSemaphore");
        exit(EXIT_FAILURE);
    }

    // Optionally, deinitialize other resources needed for the server
}


void addNewNetworkNode(NetworkNodesList *list, int nodeNum, int neighborsList[32]) {
    // Initialize node properties
    list->node[nodeNum].nodeNum = nodeNum;
    memcpy(list->node[nodeNum].neighborsList, neighborsList, sizeof(int) * 32);

    // Generate pipe name
    char pipeName[64];
    sprintf(pipeName, "%s%d", NODE_PIPE_PREFIX, nodeNum);
    strcpy(list->node[nodeNum].pipeName, pipeName);

    // Unlink existing pipe (named pipe) if it exists
    if (unlink(pipeName) == -1 && errno != ENOENT) {
        perror("unlink");
        exit(EXIT_FAILURE);
    }

    // Initialize pipe
    if (mkfifo(pipeName, 0666) == -1) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }

    // Generate semaphore names
    char mutexName[64];
    char counterName[64];
    sprintf(mutexName, "%s%d", NODE_PIPE_MUTEX_PREFIX, nodeNum);
    sprintf(counterName, "%s%d", NODE_PIPE_COUNTER_PREFIX, nodeNum);

    // Unlink existing pipe mutex semaphore if it exists
    sem_unlink(mutexName);

    // Unlink existing pipe counter semaphore if it exists
    sem_unlink(counterName);

    // Initialize pipe mutex semaphore
    sem_t *mutex_sem = sem_open(mutexName, O_CREAT, 0666, 1);
    if (mutex_sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    list->node[nodeNum].pipeMutex = mutex_sem;

    // Initialize pipe counter semaphore
    sem_t *counter_sem = sem_open(counterName, O_CREAT, 0666, 0);
    if (counter_sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    list->node[nodeNum].pipeCounter = counter_sem;

    // Optionally, set up other properties of the node
}


void deleteNetworkNode(NetworkNodesList *list, int nodeNum) {
    // Unlink the FIFO pipe
    if (unlink(list->node[nodeNum].pipeName) == -1 && errno != ENOENT) {
        perror("unlink");
        exit(EXIT_FAILURE);
    }

    // Destroy pipe mutex semaphore
    if (sem_close(list->node[nodeNum].pipeMutex) == -1) {
        perror("sem_close");
        exit(EXIT_FAILURE);
    }

    // Destroy pipe counter semaphore
    if (sem_close(list->node[nodeNum].pipeCounter) == -1) {
        perror("sem_close");
        exit(EXIT_FAILURE);
    }

    // Unlink pipe mutex semaphore
    char mutexName[64];
    sprintf(mutexName, "%s%d", NODE_PIPE_MUTEX_PREFIX, nodeNum);
    sem_unlink(mutexName);

    // Unlink pipe counter semaphore
    char counterName[64];
    sprintf(counterName, "%s%d", NODE_PIPE_COUNTER_PREFIX, nodeNum);
    sem_unlink(counterName);

    // Optionally, clear other properties or perform additional cleanup

    // Destroy the semaphores
    sem_destroy(list->node[nodeNum].pipeMutex);
    sem_destroy(list->node[nodeNum].pipeCounter);

    // Free allocated memory for the semaphores
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

    // Open the server pipe mutex
    serverPipeMutex = sem_open(SERVER_PIPE_MUTEX_NAME, 0);
    if (serverPipeMutex == SEM_FAILED) {
        perror("sem_open for serverPipeMutex");
        exit(EXIT_FAILURE);
    }

    // Open the server pipe count semaphore
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
        
        // Read individualNodeNetworkNum, msg len, and msg from the server pipe
        int senderNum;
        int len;
        read_struct(serverPipe, &senderNum, sizeof(int));
        read_struct(serverPipe, &len, sizeof(int));
        uint8_t *msg = malloc(len);
        read_struct(serverPipe, msg, len);
        
        sem_post(serverPipeMutex);
        
        // Call linuxSimulationServerPacketsHandler with received message
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
    char nodePipeName[64];  // Local variable for the node pipe name
    sem_t *nodePipeMutex;   // Local variable for the node pipe mutex
    sem_t *nodePipeCounter; // Local variable for the node pipe counter

    // Construct the node pipe name using individualNodeNetworkNum as a suffix
    sprintf(nodePipeName, "%s%d", NODE_PIPE_PREFIX, individualNodeNetworkNum);

    // Open the node pipe mutex
    char mutexName[64];
    sprintf(mutexName, "%s%d", NODE_PIPE_MUTEX_PREFIX, individualNodeNetworkNum);
    nodePipeMutex = sem_open(mutexName, 0);
    if (nodePipeMutex == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    // Open the node pipe counter
    char counterName[64];
    sprintf(counterName, "%s%d", NODE_PIPE_COUNTER_PREFIX, individualNodeNetworkNum);
    nodePipeCounter = sem_open(counterName, 0);
    if (nodePipeCounter == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    
    // Open node pipe
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

    // Open node pipe
    int nodePipe = open(node->pipeName, O_WRONLY);
    if (nodePipe < 0) {
        perror("Error opening node pipe");
        sem_post(node->pipeMutex);
        return;
    }
    
    sem_wait(node->pipeMutex);
    
    // Write length of the message to the node pipe
    write(nodePipe, &len, sizeof(int));
    
    // Write msg to the node pipe
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
