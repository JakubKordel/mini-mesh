#include "packetEncryption.h"

#include <stdlib.h>
#include <string.h>

#include "mbedtls/aes.h"

void initEncryption(EncryptionHandler *handler) {
    mbedtls_aes_init(&handler->ctx);
}

void deinitEncryption(EncryptionHandler *handler) {

}

void addPacketHash(EncryptionHandler *handler, void *packet, int len, void *packetWithHash, int *newLen) {

    int paddingBytes = AES_BLOCK_SIZE - ((len + 32) % AES_BLOCK_SIZE);
    int newPacketLen = len + paddingBytes + 32;

    memcpy(packetWithHash, packet, len);
    memset(packetWithHash + len, 0, paddingBytes);

    unsigned char hash[32]; 
    mbedtls_sha256(packetWithHash, newPacketLen - 32, hash, 0); 

    memcpy(packetWithHash + len + paddingBytes, hash, 32);

    *newLen = newPacketLen;

}

bool validatePacketHash(EncryptionHandler *handler, void *packetWithHash, int len) {
    unsigned char receivedHash[32];
    memcpy(receivedHash, packetWithHash + len - 32, 32);

    unsigned char calculatedHash[32];
    mbedtls_sha256(packetWithHash, len - 32, calculatedHash, 0); 

    return memcmp(receivedHash, calculatedHash, 32) == 0;
}

void encryptPacket(EncryptionHandler *handler, void *packet, int len) {
    unsigned char *data = (unsigned char *)packet;
    unsigned char ciphertext[AES_BLOCK_SIZE];
    
    mbedtls_aes_setkey_enc(&handler->ctx, handler->key, AES_KEY_SIZE * 8); 
    
    for (int i = 0; i < len; i += AES_BLOCK_SIZE) {
        mbedtls_aes_crypt_ecb(&handler->ctx, MBEDTLS_AES_ENCRYPT, data + i, ciphertext);
        memcpy(data + i, ciphertext, AES_BLOCK_SIZE);
    }
}

void decryptPacket(EncryptionHandler *handler, void *packet, int len) {
    unsigned char *data = (unsigned char *)packet;
    unsigned char plaintext[AES_BLOCK_SIZE];
    
    mbedtls_aes_setkey_dec(&handler->ctx, handler->key, AES_KEY_SIZE * 8);
        
    for (int i = 0; i < len; i += AES_BLOCK_SIZE) {
        mbedtls_aes_crypt_ecb(&handler->ctx, MBEDTLS_AES_DECRYPT, data + i, plaintext);
        memcpy(data + i, plaintext, AES_BLOCK_SIZE);
    }
}
