#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include "../RNG/rng.hpp"
#include "../CMA/CMA.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "encryption.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Template/pair.hpp"

const char *be_get_encryption_key(void)
{
    ft_size_t key_length;
    char *key;
    int32_t secure_error;

    key_length = 32;
    key = static_cast<char *>(cma_malloc(key_length + 1));
    if (key == ft_nullptr)
        return (ft_nullptr);
    if (cmp_rng_secure_bytes(reinterpret_cast<uint8_t *>(key),
            key_length) == FT_ERR_SUCCESS)
    {
        ft_size_t index;

        index = 0;
        while (index < key_length)
        {
            key[index] = static_cast<char>('A'
                + (static_cast<uint8_t>(key[index]) % 26));
            ++index;
        }
        key[key_length] = '\0';
        return (key);
    }
    secure_error = FT_ERR_INTERNAL;
    uint32_t seed_value = ft_random_seed(ft_nullptr);
    ft_size_t index;

    index = 0;
    while (index < key_length)
    {
        seed_value = seed_value * 1103515245u + 12345u;
        key[index] = static_cast<char>('A' + (seed_value % 26u));
        ++index;
    }
    key[key_length] = '\0';
    if (secure_error != FT_ERR_SUCCESS)
        return (key);
    return (key);
}
