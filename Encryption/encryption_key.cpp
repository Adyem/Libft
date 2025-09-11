#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#ifndef _WIN32
# include <unistd.h>
#else
# include <windows.h>
# include <wincrypt.h>
#endif
#include "../RNG/rng.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "basic_encryption.hpp"

const char *be_getEncryptionKey(void)
{
    size_t key_length = 32;
    char *key = static_cast<char *>(cma_malloc(key_length + 1));
    if (!key)
        return (ft_nullptr);

#ifdef _WIN32
    HCRYPTPROV crypto_provider;
    if (CryptAcquireContext(&crypto_provider, ft_nullptr, ft_nullptr,
            PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
    {
        if (CryptGenRandom(crypto_provider, static_cast<DWORD>(key_length),
                reinterpret_cast<BYTE *>(key)))
        {
            CryptReleaseContext(crypto_provider, 0);
            size_t index = 0;
            while (index < key_length)
            {
                key[index] = static_cast<char>('A'
                    + (static_cast<unsigned char>(key[index]) % 26));
                index++;
            }
            key[key_length] = '\0';
            return (key);
        }
        CryptReleaseContext(crypto_provider, 0);
    }
#else
    int file_descriptor = open("/dev/urandom", O_RDONLY);
    if (file_descriptor != -1)
    {
        ssize_t read_result = read(file_descriptor, key, key_length);
        close(file_descriptor);
        if (read_result == static_cast<ssize_t>(key_length))
        {
            size_t index = 0;
            while (index < key_length)
            {
                key[index] = static_cast<char>('A'
                    + (static_cast<unsigned char>(key[index]) % 26));
                index++;
            }
            key[key_length] = '\0';
            return (key);
        }
    }
#endif
    unsigned int seed_value = static_cast<unsigned int>(ft_random_seed(ft_nullptr));
    size_t index = 0;
    while (index < key_length)
    {
        seed_value = seed_value * 1103515245u + 12345u;
        key[index] = static_cast<char>('A' + (seed_value % 26u));
        index++;
    }
    key[key_length] = '\0';
    return (key);
}

