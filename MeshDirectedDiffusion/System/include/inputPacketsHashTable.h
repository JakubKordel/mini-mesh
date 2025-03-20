#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdbool.h>
#include <stdint.h>
#include <universal_semaphore.h>

#define TABLE_SIZE 64
#define HASH_SIZE 32
#define BOXES 8

typedef struct {
    uint8_t hash[HASH_SIZE];
} HashEntry;

typedef struct {
    HashEntry table[TABLE_SIZE];
    uint32_t nextInsertPtr[BOXES]; 
	binary_semaphore_t * mutex;
} HashTable;

HashTable* hash_table_init();

void hash_table_deinit(HashTable *ht);

bool hash_table_insert(HashTable *ht, const uint8_t *key);
bool hash_table_exists(HashTable *ht, const uint8_t *key);

#endif  // HASH_TABLE_H