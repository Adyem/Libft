#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "compatebility_internal.hpp"

#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
# include <wincrypt.h>
int cmp_rng_secure_bytes(unsigned char *buffer, size_t length)
{
    HCRYPTPROV crypt_provider = 0;
    if (CryptAcquireContext(&crypt_provider, ft_nullptr, ft_nullptr,
            PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) == 0)
    {
        DWORD last_error = GetLastError();
        if (last_error != 0)
            ft_errno = static_cast<int>(last_error) + ERRNO_OFFSET;
        else
            ft_errno = FT_EINVAL;
        return (-1);
    }
    if (CryptGenRandom(crypt_provider, static_cast<DWORD>(length), buffer) == 0)
    {
        DWORD last_error = GetLastError();
        CryptReleaseContext(crypt_provider, 0);
        if (last_error != 0)
            ft_errno = static_cast<int>(last_error) + ERRNO_OFFSET;
        else
            ft_errno = FT_EINVAL;
        return (-1);
    }
    if (CryptReleaseContext(crypt_provider, 0) == 0)
    {
        DWORD last_error = GetLastError();
        if (last_error != 0)
            ft_errno = static_cast<int>(last_error) + ERRNO_OFFSET;
        else
            ft_errno = FT_EINVAL;
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}
#else
# include <unistd.h>
# include <fcntl.h>
# include <cerrno>

void cmp_force_rng_open_failure(int error_code);
void cmp_force_rng_read_failure(int error_code);
void cmp_force_rng_read_eof(void);
void cmp_force_rng_close_failure(int error_code);
void cmp_clear_force_rng_failures(void);

static int g_force_rng_open_errno = 0;
static int g_force_rng_read_errno = 0;
static int g_force_rng_close_errno = 0;
static int g_force_rng_read_zero = 0;

void cmp_force_rng_open_failure(int error_code)
{
    g_force_rng_open_errno = error_code;
    return ;
}

void cmp_force_rng_read_failure(int error_code)
{
    g_force_rng_read_errno = error_code;
    return ;
}

void cmp_force_rng_close_failure(int error_code)
{
    g_force_rng_close_errno = error_code;
    return ;
}

void cmp_force_rng_read_eof(void)
{
    g_force_rng_read_zero = 1;
    return ;
}

void cmp_clear_force_rng_failures(void)
{
    g_force_rng_open_errno = 0;
    g_force_rng_read_errno = 0;
    g_force_rng_close_errno = 0;
    g_force_rng_read_zero = 0;
    return ;
}
int cmp_rng_secure_bytes(unsigned char *buffer, size_t length)
{
    int forced_open_errno = g_force_rng_open_errno;
    g_force_rng_open_errno = 0;
    if (forced_open_errno != 0)
    {
        errno = forced_open_errno;
        ft_errno = errno + ERRNO_OFFSET;
        return (-1);
    }
    int file_descriptor = open("/dev/urandom", O_RDONLY);
    if (file_descriptor < 0)
    {
        ft_errno = errno + ERRNO_OFFSET;
        return (-1);
    }
    size_t offset = 0;
    while (offset < length)
    {
        int forced_read_errno = g_force_rng_read_errno;
        if (forced_read_errno != 0)
        {
            g_force_rng_read_errno = 0;
            errno = forced_read_errno;
            ft_errno = errno + ERRNO_OFFSET;
            int stored_errno = errno;
            close(file_descriptor);
            errno = stored_errno;
            return (-1);
        }
        ssize_t bytes_read;

        if (g_force_rng_read_zero != 0)
        {
            g_force_rng_read_zero = 0;
            bytes_read = 0;
        }
        else
        {
            bytes_read = read(file_descriptor, buffer + offset,
                length - offset);
        }
        if (bytes_read < 0)
        {
            ft_errno = errno + ERRNO_OFFSET;
            int stored_errno = errno;
            close(file_descriptor);
            errno = stored_errno;
            return (-1);
        }
        if (bytes_read == 0)
        {
            int close_result = close(file_descriptor);

            if (close_result < 0)
            {
                ft_errno = errno + ERRNO_OFFSET;
                return (-1);
            }
            ft_errno = FT_EIO;
            return (-1);
        }
        offset += static_cast<size_t>(bytes_read);
    }
    int close_result = close(file_descriptor);
    if (close_result < 0)
    {
        ft_errno = errno + ERRNO_OFFSET;
        return (-1);
    }
    int forced_close_errno = g_force_rng_close_errno;
    g_force_rng_close_errno = 0;
    if (forced_close_errno != 0)
    {
        errno = forced_close_errno;
        ft_errno = errno + ERRNO_OFFSET;
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}
#endif
