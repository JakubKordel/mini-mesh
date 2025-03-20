#ifndef PACKETENCRYPTION_H_
#define PACKETENCRYPTION_H_

#include <stdbool.h>
#include "mbedtls/platform.h"
#include "mbedtls/aes.h"
#include "mbedtls/sha256.h"

#define AES_KEY_SIZE    16 
#define AES_BLOCK_SIZE  16 

typedef struct {
    mbedtls_aes_context ctx;
    unsigned char key[AES_KEY_SIZE + 1];
} EncryptionHandler;

void initEncryption(EncryptionHandler *handler);

void deinitEncryption(EncryptionHandler *handler);

void addPacketHash(EncryptionHandler *handler, void *packet, int len, void *packetWithHash, int *newLen);

bool validatePacketHash(EncryptionHandler *handler, void *packet, int len);

void encryptPacket(EncryptionHandler *handler, void *packet, int len);

void decryptPacket(EncryptionHandler *handler, void *packet, int len);

#endif // PACKETENCRYPTION_H_
