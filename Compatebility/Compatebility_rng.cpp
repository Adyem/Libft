#include "../CPP_class/class_nullptr.hpp"
#include "compatebility_internal.hpp"

#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
# include <wincrypt.h>
int cmp_rng_secure_bytes(unsigned char *buffer, size_t length)
{
    HCRYPTPROV crypt_provider = 0;
    if (CryptAcquireContext(&crypt_provider, ft_nullptr, ft_nullptr,
            PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) == 0)
        return (-1);
    if (CryptGenRandom(crypt_provider, static_cast<DWORD>(length), buffer) == 0)
    {
        CryptReleaseContext(crypt_provider, 0);
        return (-1);
    }
    CryptReleaseContext(crypt_provider, 0);
    return (0);
}
#else
# include <unistd.h>
# include <fcntl.h>
int cmp_rng_secure_bytes(unsigned char *buffer, size_t length)
{
    int file_descriptor = open("/dev/urandom", O_RDONLY);
    if (file_descriptor < 0)
        return (-1);
    size_t offset = 0;
    while (offset < length)
    {
        ssize_t bytes_read = read(file_descriptor, buffer + offset,
            length - offset);
        if (bytes_read < 0)
        {
            close(file_descriptor);
            return (-1);
        }
        offset += static_cast<size_t>(bytes_read);
    }
    close(file_descriptor);
    return (0);
}
#endif
