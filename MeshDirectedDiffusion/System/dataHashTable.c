#include "dataHashTable.h"
#include <string.h>
#include <stdlib.h>

DataHashTable * dataHashTableInit() {
    DataHashTable *dht = (DataHashTable*)malloc(sizeof(DataHashTable));
    if (dht == NULL) {
        return NULL;
    }
	
    for (uint32_t i = 0; i < DATA_HASH_TABLE_SIZE; i++) {
        memset(dht->table[i].hash, 0, DATA_HASH_SIZE);
    }

    for (uint32_t i = 0; i < BOXES_NUM; i++) {
        dht->nextInsertPtr[i] = i;
    }
	
	dht->mutex = binary_semaphore_create(1);
	
    return dht;
}

void dataHashTableDeinit(DataHashTable *dht) {
	
	binary_semaphore_destroy(dht->mutex);

    free(dht);  
}

void dataHashTableInsert(DataHashTable *dht, uint8_t hash[DATA_HASH_SIZE]) {
    binary_semaphore_wait(dht->mutex);
	
    uint32_t boxIndex = hash[0] % BOXES_NUM;
    uint32_t insertIndex = dht->nextInsertPtr[boxIndex];
    memcpy(dht->table[insertIndex].hash, hash, DATA_HASH_SIZE);

    dht->nextInsertPtr[boxIndex] = (insertIndex + BOXES_NUM) % DATA_HASH_TABLE_SIZE;

    binary_semaphore_post(dht->mutex);
}

bool dataHashTableExists(DataHashTable *dht, uint8_t hash[DATA_HASH_SIZE]) {
    binary_semaphore_wait(dht->mutex);

    uint32_t boxIndex = hash[0] % BOXES_NUM;

    for (uint32_t i = boxIndex; i < DATA_HASH_TABLE_SIZE; i += BOXES_NUM) {
        if (memcmp(dht->table[i].hash, hash, DATA_HASH_SIZE) == 0) {
            binary_semaphore_post(dht->mutex);
            return true;
        }
    }

    binary_semaphore_post(dht->mutex);
    return false;
}