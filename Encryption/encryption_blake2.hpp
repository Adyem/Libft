#ifndef ENCRYPTION_BLAKE2_HPP
#define ENCRYPTION_BLAKE2_HPP

#include <stddef.h>

void blake2b_hash(const void *data, size_t length, unsigned char *digest, size_t digest_length);
void blake2s_hash(const void *data, size_t length, unsigned char *digest, size_t digest_length);

#endif
