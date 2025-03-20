#include "interestCallbackTable.h"
#include <meshSystemConfig.h>

int initInterestCallbackTable(InterestCallbackTable **ict) {
    if (ict == NULL) {
        return -1; 
    }
    
    *ict = (InterestCallbackTable*)malloc(sizeof(InterestCallbackTable));
    if (*ict == NULL) {
        return -1; 
    }
    
    (*ict)->mutex = binary_semaphore_create(1);
    (*ict)->firstUsed = MAX_INTEREST_CALLBACKS_NUM; 

    return 0;
}

int addInterestCallback(InterestCallbackTable *ict, InterestId interestId, void (*interestCallback)(void *params), void *params) {
    if (ict == NULL || interestCallback == NULL) {
        return -1;
    }

    binary_semaphore_wait(ict->mutex);

    int emptyNode = MAX_INTEREST_CALLBACKS_NUM;
    int i = ict->firstUsed;

    if (i > 0) {
        emptyNode = 0;
    }

    while (i != MAX_INTEREST_CALLBACKS_NUM) {
        if (compareInterestId(&ict->interestCallback[i].interestId, &interestId) == 0) {
            binary_semaphore_post(ict->mutex);
            return 0;
        }

        if (emptyNode == MAX_INTEREST_CALLBACKS_NUM && ict->interestCallback[i].next - i > 1) {
            emptyNode = i + 1;
        }

        i = ict->interestCallback[i].next;
    }

    if (emptyNode != MAX_INTEREST_CALLBACKS_NUM) {
        if (emptyNode != 0) {
            ict->interestCallback[emptyNode].next = ict->interestCallback[emptyNode - 1].next;
            ict->interestCallback[emptyNode - 1].next = emptyNode;
        } else {
            ict->interestCallback[emptyNode].next = ict->firstUsed;
            ict->firstUsed = 0;
        }

        ict->interestCallback[emptyNode].interestId = interestId;
        ict->interestCallback[emptyNode].interestCallback = interestCallback;
        ict->interestCallback[emptyNode].params = params;

        binary_semaphore_post(ict->mutex);
        return 0;
    }

    binary_semaphore_post(ict->mutex);
    return -1;
}

int getInterestCallback(InterestCallbackTable *ict, InterestId interestId, void (**interestCallback)(void *params), void **params) {
    if (ict == NULL || interestCallback == NULL || params == NULL) {
        return -2;
    }

    binary_semaphore_wait(ict->mutex);

    int i = ict->firstUsed;

    while (i != MAX_INTEREST_CALLBACKS_NUM) {
        if (compareInterestId(&ict->interestCallback[i].interestId, &interestId) == 0) {
            *interestCallback = ict->interestCallback[i].interestCallback;
            *params = ict->interestCallback[i].params;
            binary_semaphore_post(ict->mutex);
            return 0;
        }

        i = ict->interestCallback[i].next;
    }

    binary_semaphore_post(ict->mutex);
    return -1;
}

int removeInterestCallback(InterestCallbackTable *ict, InterestId interestId) {
    if (ict == NULL) {
        return -1;
    }

    binary_semaphore_wait(ict->mutex);

    int i = ict->firstUsed;
    int prev = MAX_INTEREST_CALLBACKS_NUM;

    while (i != MAX_INTEREST_CALLBACKS_NUM) {
        if (compareInterestId(&ict->interestCallback[i].interestId, &interestId) == 0) {
            if (prev != MAX_INTEREST_CALLBACKS_NUM) {
                ict->interestCallback[prev].next = ict->interestCallback[i].next;
            } else {
                ict->firstUsed = ict->interestCallback[i].next;
            }

            binary_semaphore_post(ict->mutex);
            return 0;
        }

        prev = i;
        i = ict->interestCallback[i].next;
    }

    binary_semaphore_post(ict->mutex);
    return -1;
}

int freeInterestCallbackTable(InterestCallbackTable * ict) {
	
	binary_semaphore_destroy(ict->mutex);
	
    free(ict);
    return 0;
}