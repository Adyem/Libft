#ifndef BASIC_ENCRYPTION_HPP
#define BASIC_ENCRYPTION_HPP

#include "aes.hpp"
#include "rsa.hpp"
#include "encryption_sha256.hpp"
#include "encryption_hmac_sha256.hpp"

int            be_saveGame(const char *filename, const char *data, const char *key);
char        **be_DecryptData(char **data, const char *key);
const char    *be_getEncryptionKey();



#endif
