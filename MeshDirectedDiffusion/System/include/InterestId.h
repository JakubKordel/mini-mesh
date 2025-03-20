#ifndef INTERESTID_H_
#define INTERESTID_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint32_t id[4];
} InterestId;

int makeInterestId(InterestId *interestId, uint32_t first4Bytes, uint32_t second4Bytes, uint32_t third4Bytes, uint32_t fourth4Bytes); 

int compareInterestId(InterestId * id1, InterestId * id2);

#endif /* INTERESTID_H_ */