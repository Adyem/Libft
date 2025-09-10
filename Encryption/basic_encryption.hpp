#ifndef BASIC_ENCRYPTION_HPP
#define BASIC_ENCRYPTION_HPP

#include "aes.hpp"

int            be_saveGame(const char *filename, const char *data, const char *key);
char        **be_DecryptData(char **data, const char *key);
const char    *be_getEncryptionKey();

/*
Usage:
    unsigned char block[16] = {0};
    unsigned char key[16] = {0};
    aes_encrypt(block, key);
    aes_decrypt(block, key);
*/

#endif
