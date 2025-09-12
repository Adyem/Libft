#ifndef ENCRYPTION_HMAC_SHA256_HPP
#define ENCRYPTION_HMAC_SHA256_HPP

#include <stddef.h>

void hmac_sha256(const unsigned char *key, size_t key_len, const void *data, size_t len, unsigned char *digest);

#endif
