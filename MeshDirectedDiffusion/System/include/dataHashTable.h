#ifndef DATA_HASH_TABLE_H
#define DATA_HASH_TABLE_H

#include <stdbool.h>
#include <stdint.h>
#include <universal_semaphore.h>

#define DATA_HASH_TABLE_SIZE 128
#define DATA_HASH_SIZE 32
#define BOXES_NUM 16

typedef struct {
    uint8_t hash[DATA_HASH_SIZE];
} DataHashTableEntry;

typedef struct {
    DataHashTableEntry table[DATA_HASH_TABLE_SIZE];
    uint32_t nextInsertPtr[BOXES_NUM]; 
	binary_semaphore_t * mutex;
} DataHashTable;

DataHashTable * dataHashTableInit();

void dataHashTableDeinit(DataHashTable *dht);

void dataHashTableInsert(DataHashTable *dht, uint8_t hash[DATA_HASH_SIZE]);
bool dataHashTableExists(DataHashTable *dht, uint8_t hash[DATA_HASH_SIZE]);

#endif  // DATA_HASH_TABLE_H
