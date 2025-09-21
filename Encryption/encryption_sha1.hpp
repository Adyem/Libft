#ifndef ENCRYPTION_SHA1_HPP
#define ENCRYPTION_SHA1_HPP

#include <stddef.h>

void sha1_hash(const void *data, size_t length, unsigned char *digest);

#endif
