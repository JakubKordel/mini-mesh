#ifndef DATARECEIVEDCALLBACKTABLE_H_
#define DATARECEIVEDCALLBACKTABLE_H_

#include <InterestId.h>
#include <universal_semaphore.h>
#include <meshSystemConfig.h>

typedef struct {
    InterestId interestId;
    void (*dataCallback)(void*, int);
	uint8_t maxDist;
    int next;
} DataCallback;

typedef struct DataReceivedCallbackTable {
    DataCallback dataCallback[MAX_DATA_CALLBACKS_NUM];
    int firstUsed; 
    binary_semaphore_t *mutex;
} DataReceivedCallbackTable;

int initDataReceivedCallbackTable(DataReceivedCallbackTable **drct);
int addDataCallback(DataReceivedCallbackTable *drct, InterestId interestId, void (*dataCallback)(void*, int), uint8_t maxDist);
int getDataCallback(DataReceivedCallbackTable *drct, InterestId interestId, void (**dataCallback)(void*, int));
int removeDataCallback(DataReceivedCallbackTable *drct, InterestId interestId);
int freeDataCallbackTable(DataReceivedCallbackTable *drct);

#endif /* DATARECEIVEDCALLBACKTABLE_H_ */
