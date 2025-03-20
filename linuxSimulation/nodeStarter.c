#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simulServer/simulServer.h"
#include "simulNodes/simulNodes.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <node_num> <task>\n", argv[0]);
        return -1;
    }

    individualNodeNetworkNum = atoi(argv[1]);
    char *task = argv[2];

    if (strcmp(task, "relayingNode") == 0) {
    	linuxSimulationPrint("Starting Relaying Node\n" ) ;
    	relayingNode();
    } else if (strcmp(task, "receiverNode") == 0) {
    	linuxSimulationPrint("Starting Receiver Node\n" ) ;
        receiverNode();
    } else if (strcmp(task, "senderNode") == 0) {
    	linuxSimulationPrint("Starting Sender Node\n" ) ;
        senderNode();
    } else if (strcmp(task, "mirrorNode") == 0) {
    	linuxSimulationPrint("Starting Mirror Node\n" ) ;
        mirrorNode();
    } else if (strcmp(task, "toMirrorSenderNode") == 0) {
    	linuxSimulationPrint("Starting To Mirror Sender Node\n" ) ;
        toMirrorSenderNode();
    } else {
        fprintf(stderr, "Unknown task: %s\n", task);
        return -1;
    }

    return 0;
}


