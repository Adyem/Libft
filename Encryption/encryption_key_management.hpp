#ifndef ENCRYPTION_KEY_MANAGEMENT_HPP
#define ENCRYPTION_KEY_MANAGEMENT_HPP

#include <cstddef>

unsigned char    *encryption_generate_symmetric_key(size_t key_length);
unsigned char    *encryption_generate_initialization_vector(size_t iv_length);
int                encryption_fill_secure_buffer(unsigned char *buffer, size_t buffer_length);

#endif
