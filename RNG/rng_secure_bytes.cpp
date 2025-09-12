#include "rng.hpp"

#ifdef _WIN32
# include <windows.h>
# include <wincrypt.h>
#else
# include <unistd.h>
# include <fcntl.h>
#endif

int rng_secure_bytes(unsigned char *buffer, size_t length)
{
    if (buffer == ft_nullptr)
        return (-1);
#ifdef _WIN32
    HCRYPTPROV crypt_provider = 0;
    if (CryptAcquireContext(&crypt_provider, ft_nullptr, ft_nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) == 0)
        return (-1);
    if (CryptGenRandom(crypt_provider, static_cast<DWORD>(length), buffer) == 0)
    {
        CryptReleaseContext(crypt_provider, 0);
        return (-1);
    }
    CryptReleaseContext(crypt_provider, 0);
    return (0);
#else
    int file_descriptor = open("/dev/urandom", O_RDONLY);
    if (file_descriptor < 0)
        return (-1);
    size_t offset = 0;
    while (offset < length)
    {
        ssize_t bytes_read = read(file_descriptor, buffer + offset, length - offset);
        if (bytes_read < 0)
        {
            close(file_descriptor);
            return (-1);
        }
        offset += static_cast<size_t>(bytes_read);
    }
    close(file_descriptor);
    return (0);
#endif
}

uint32_t ft_random_uint32(void)
{
    unsigned char byte_buffer[4];
    if (rng_secure_bytes(byte_buffer, 4) != 0)
        return (0);
    uint32_t random_value = 0;
    size_t index = 0;
    while (index < 4)
    {
        random_value |= static_cast<uint32_t>(byte_buffer[index]) << (index * 8);
        index++;
    }
    return (random_value);
}

