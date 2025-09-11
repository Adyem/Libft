#ifndef BASIC_ENCRYPTION_HPP
#define BASIC_ENCRYPTION_HPP

#include "aes.hpp"
#include "rsa.hpp"

int            be_saveGame(const char *filename, const char *data, const char *key);
char        **be_DecryptData(char **data, const char *key);
const char    *be_getEncryptionKey();

/*
Usage:
    unsigned char block[16] = {0};
    unsigned char key[16] = {0};
    aes_encrypt(block, key);
    aes_decrypt(block, key);

    uint64_t public_key;
    uint64_t private_key;
    uint64_t modulus;
    rsa_generate_key_pair(&public_key, &private_key, &modulus, 32);
    uint64_t encrypted = rsa_encrypt(42, public_key, modulus);
    uint64_t decrypted = rsa_decrypt(encrypted, private_key, modulus);
*/

#endif
