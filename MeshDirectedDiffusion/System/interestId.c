#include "InterestId.h"

int makeInterestId(InterestId *interestId, uint32_t first4Bytes, uint32_t second4Bytes, uint32_t third4Bytes, uint32_t fourth4Bytes){
	if (interestId != NULL){
		memcpy(&interestId->id[0], &first4Bytes, sizeof(uint32_t));
		memcpy(&interestId->id[1], &second4Bytes, sizeof(uint32_t));
		memcpy(&interestId->id[2], &third4Bytes, sizeof(uint32_t));
		memcpy(&interestId->id[3], &fourth4Bytes, sizeof(uint32_t));
		return 0;
	}
	return -1;
}

int compareInterestId(InterestId * id1, InterestId * id2) {
    for (int i = 0; i < 4; i++) {
        if (id1->id[i] != id2->id[i]) {
            return id1->id[i] - id2->id[i];
        }
    }
    return 0;
}
