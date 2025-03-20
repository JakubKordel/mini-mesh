#ifndef INTERESTCALLBACKTABLE_H_
#define INTERESTCALLBACKTABLE_H_

#include <InterestId.h>
#include <stdbool.h>
#include <universal_semaphore.h>
#include <meshSystemConfig.h>

typedef struct {
	InterestId interestId;
	void (*interestCallback)(void * params);
	void * params;
	int next;
} InterestCallback;

typedef struct InterestCallbackTable {
    InterestCallback interestCallback[MAX_INTEREST_CALLBACKS_NUM];
	binary_semaphore_t * mutex;
	int firstUsed; 
} InterestCallbackTable;

int initInterestCallbackTable(InterestCallbackTable ** ict);
int addInterestCallback(InterestCallbackTable * ict, InterestId interestId, void (*interestCallback)(void * params ), void * params);
int getInterestCallback(InterestCallbackTable * ict, InterestId interestId, void (**interestCallback)(void * params), void ** params);
int removeInterestCallback(InterestCallbackTable * ict, InterestId interestId);
int freeInterestCallbackTable(InterestCallbackTable * ict);


#endif /* INTERESTCALLBACKTABLE_H_ */
