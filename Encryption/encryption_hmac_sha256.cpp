#include <stddef.h>
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "encryption_hmac_sha256.hpp"
#include "encryption_sha256.hpp"

void hmac_sha256(const unsigned char *key, size_t key_len, const void *data, size_t len, unsigned char *digest)
{
    unsigned char key_block[64];
    unsigned char inner_pad[64];
    unsigned char outer_pad[64];
    unsigned char hashed_key[32];
    size_t current_index;

    if (key_len > 64)
    {
        sha256_hash(key, key_len, hashed_key);
        key = hashed_key;
        key_len = 32;
    }
    current_index = 0;
    while (current_index < 64)
    {
        unsigned char key_byte = 0;
        if (current_index < key_len)
            key_byte = key[current_index];
        key_block[current_index] = key_byte;
        inner_pad[current_index] = key_block[current_index] ^ 0x36;
        outer_pad[current_index] = key_block[current_index] ^ 0x5c;
        ++current_index;
    }
    unsigned char *inner_data = static_cast<unsigned char *>(cma_malloc(64 + len));
    if (!inner_data)
    {
        ft_errno = FT_EALLOC;
        return ;
    }
    current_index = 0;
    while (current_index < 64)
    {
        inner_data[current_index] = inner_pad[current_index];
        ++current_index;
    }
    size_t data_index = 0;
    const unsigned char *byte_data = static_cast<const unsigned char *>(data);
    while (data_index < len)
    {
        inner_data[64 + data_index] = byte_data[data_index];
        ++data_index;
    }
    unsigned char inner_digest[32];
    sha256_hash(inner_data, 64 + len, inner_digest);
    cma_free(inner_data);
    unsigned char *outer_data = static_cast<unsigned char *>(cma_malloc(64 + 32));
    if (!outer_data)
    {
        ft_errno = FT_EALLOC;
        return ;
    }
    current_index = 0;
    while (current_index < 64)
    {
        outer_data[current_index] = outer_pad[current_index];
        ++current_index;
    }
    current_index = 0;
    while (current_index < 32)
    {
        outer_data[64 + current_index] = inner_digest[current_index];
        ++current_index;
    }
    sha256_hash(outer_data, 96, digest);
    cma_free(outer_data);
    ft_errno = ER_SUCCESS;
    return ;
}
