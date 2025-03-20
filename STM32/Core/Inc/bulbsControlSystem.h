#ifndef BULBSCONTROLSYSTEM_H_
#define BULBSCONTROLSYSTEM_H_

#include <InterestId.h>

#define RECOGNITION_ID_PART_1 0x00000000
#define RECOGNITION_ID_PART_2 0x00000000
#define RECOGNITION_ID_PART_3 0x00000000
#define RECOGNITION_ID_PART_4 0x00000001

#define ENCRYPTION_KEY { \
    0x01, 0x23, 0x45, 0x67, \
    0x89, 0xAB, 0xCD, 0xEF, \
    0x12, 0x34, 0x56, 0x78, \
    0x9A, 0xBC, 0xDE, 0xF0 \
}

#define AES_KEY_LENGTH 16

typedef struct BulbInfo {
	InterestId switchId;
	uint32_t name;
	uint32_t sequenceNum;
	uint8_t state;
} BulbInfo;

typedef struct SwitchRequest {
	uint32_t sequenceNum;
} SwitchRequest;

#endif /* BULBSCONTROLSYSTEM_H_ */
