#include "inputPacketsHashTable.h"
#include <string.h>
#include <stdlib.h>

static uint32_t hash(const uint8_t *key, size_t key_size) {
    uint32_t sum = 0;
    for (size_t i = 0; i < key_size; i++) {
        sum += key[i];
    }
    return sum % TABLE_SIZE;
}

HashTable* hash_table_init() {
    HashTable *ht = (HashTable*)malloc(sizeof(HashTable));
    if (ht == NULL) {
        return NULL;
    }
	
    for (uint32_t i = 0; i < TABLE_SIZE; i++) {
        memset(ht->table[i].hash, 0, HASH_SIZE);
    }

    for (uint32_t i = 0; i < BOXES; i++) {
        ht->nextInsertPtr[i] = i;
    }
	
	ht->mutex = binary_semaphore_create(1);
	
    return ht;
}

void hash_table_deinit(HashTable *ht) {
	
	binary_semaphore_destroy(ht->mutex);

    free(ht);  
}

bool hash_table_insert(HashTable *ht, const uint8_t *key) {
	
	binary_semaphore_wait(ht->mutex);
	
    uint32_t boxIndex = hash(key, HASH_SIZE) % BOXES;
    uint32_t insertIndex = ht->nextInsertPtr[boxIndex];
    memcpy(ht->table[insertIndex].hash, key, HASH_SIZE);

    ht->nextInsertPtr[boxIndex] = (insertIndex + BOXES) % TABLE_SIZE;

	binary_semaphore_post(ht->mutex);
    return true;
}

bool hash_table_exists(HashTable *ht, const uint8_t *key) {
	
	binary_semaphore_wait(ht->mutex);
	
    uint32_t boxIndex = hash(key, HASH_SIZE) % BOXES;

    for (uint32_t i = boxIndex; i < TABLE_SIZE; i += BOXES) {
        if (memcmp(ht->table[i].hash, key, HASH_SIZE) == 0) {
			binary_semaphore_post(ht->mutex);
            return true; 
        }
    }

	binary_semaphore_post(ht->mutex);
    return false;
}