#ifndef ENCRYPTION_SHA3_HPP
#define ENCRYPTION_SHA3_HPP

#include <stddef.h>

void sha3_256_hash(const void *data, size_t length, unsigned char *digest);
void sha3_512_hash(const void *data, size_t length, unsigned char *digest);

#endif
