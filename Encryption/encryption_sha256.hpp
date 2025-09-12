#ifndef ENCRYPTION_SHA256_HPP
#define ENCRYPTION_SHA256_HPP

#include <stddef.h>

void sha256_hash(const void *data, size_t length, unsigned char *digest);

#endif
