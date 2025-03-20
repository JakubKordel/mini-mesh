(defun generate-prefix (num-nodes)
  (format nil "
#include \"simulServer/simulServer.h\"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h> 

#define NODES_NUM ~a

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
          
" num-nodes))

(defun generate-middle (input-data)
  (let ((c-code ""))
    (dolist (node input-data)
      (let* ((node-id (first node))
             (neighbors (second node))
             (task (third node))
             (neighbors-code 
              (loop for i below (length neighbors)
                    collect (format nil "neighborsList[~a] = ~a;" i (nth i neighbors))))
             (c-code-node (format nil
                                   "    memset(neighborsList, -1, sizeof(neighborsList));
    ~{~a~}
    addNewNetworkNode(&nodesList, ~a, neighborsList);
    strcpy(taskForNodeName[~a], \"~a\");~%~%"
                                   (mapcar #'identity neighbors-code) node-id node-id task)))
        (setf c-code (concatenate 'string c-code c-code-node))))
    c-code))

(defun generate-suffix (num-nodes)
  (format nil "    // Start network nodes
    for (int i = 0; i < NODES_NUM; ++i) {
        pid_t pid = fork();
        if (pid == -1) {
            return -1;
        } else if (pid == 0) {
            char nodeNumArg[16];
            snprintf(nodeNumArg, sizeof(nodeNumArg), \"%d\", i);
            char *args[] = {\"./NodeStarter\", nodeNumArg, taskForNodeName[i], NULL};
            execvp(\"./NodeStarter\", args);
            perror(\"execvp\");
            return -1;
        } else {
            childProcesses[i] = pid;
        }
    }

    linuxSimulationServerReceiveProcess(&nodesList);

    return 0;
}
"))

(defun generate-c-code (input-data)
  (let* ((num-nodes (length input-data))
         (prefix (generate-prefix num-nodes))
         (middle (generate-middle input-data))
         (suffix (generate-suffix num-nodes)))
    (format t "Generated C code:~%~a~a~a" prefix middle suffix)))
    
    

(defvar *input-data*
  '(	
	;; (individualNodeId (neighborId0 neighborId1 neighborId2 ... ) "taskToDoName")
	(0 (1) "relayingNode")
	(1 (0 2) "senderNode")
	(2 (1 4) "relayingNode")
	(3 (4) "relayingNode")
	(4 (2 3 5) "relayingNode")
	(5 (4 6 9) "relayingNode")
	(6 (4 5 7 9 10) "receiverNode")
	(7 (6 9) "relayingNode")
	(8 (9) "relayingNode")
	(9 (5 6 7 8) "senderNode")
	(10 (6 11) "relayingNode")
	(11 (10 12 13) "relayingNode")
	(12 (11 14) "relayingNode")
	(13 (11) "relayingNode")
	(14 (12) "relayingNode")
    )
)

;; Generate C code
(generate-c-code *input-data*)


