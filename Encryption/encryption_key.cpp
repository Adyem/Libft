#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include "../RNG/rng.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "basic_encryption.hpp"
#include "../Compatebility/compatebility_internal.hpp"

const char *be_getEncryptionKey(void)
{
    size_t key_length = 32;
    char *key = static_cast<char *>(cma_malloc(key_length + 1));
    if (key == ft_nullptr)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    if (cmp_rng_secure_bytes(reinterpret_cast<unsigned char *>(key), key_length) == 0)
    {
        size_t index = 0;
        while (index < key_length)
        {
            key[index] = static_cast<char>('A'
                + (static_cast<unsigned char>(key[index]) % 26));
            index++;
        }
        key[key_length] = '\0';
        ft_errno = FT_ER_SUCCESSS;
        return (key);
    }
    int secure_error = ft_errno;
    uint32_t seed_value = ft_random_seed(ft_nullptr);
    size_t index = 0;
    while (index < key_length)
    {
        seed_value = seed_value * 1103515245u + 12345u;
        key[index] = static_cast<char>('A' + (seed_value % 26u));
        index++;
    }
    key[key_length] = '\0';
    if (secure_error != FT_ER_SUCCESSS)
    {
        ft_errno = secure_error;
        return (key);
    }
    ft_errno = FT_ER_SUCCESSS;
    return (key);
}
