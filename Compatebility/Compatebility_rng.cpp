#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "compatebility_internal.hpp"

#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
# include <wincrypt.h>

static int32_t cmp_rng_windows_error(DWORD last_error)
{
    if (last_error != 0)
    {
        return (ft_map_system_error(static_cast<int32_t>(last_error)));
    }
    return (FT_ERR_INVALID_ARGUMENT);
}

int32_t cmp_rng_secure_bytes(unsigned char *buffer, ft_size_t length)
{
    HCRYPTPROV crypt_provider = 0;
    int32_t error_code;

    (void)buffer;
    (void)length;
    if (CryptAcquireContext(&crypt_provider, ft_nullptr, ft_nullptr,
            PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) == 0)
    {
        error_code = cmp_rng_windows_error(GetLastError());
        cmp_set_last_error(error_code);
        return (-1);
    }
    if (CryptGenRandom(crypt_provider, static_cast<DWORD>(length), buffer) == 0)
    {
        error_code = cmp_rng_windows_error(GetLastError());
        CryptReleaseContext(crypt_provider, 0);
        cmp_set_last_error(error_code);
        return (-1);
    }
    if (CryptReleaseContext(crypt_provider, 0) == 0)
    {
        error_code = cmp_rng_windows_error(GetLastError());
        cmp_set_last_error(error_code);
        return (-1);
    }
    cmp_set_last_error(FT_ERR_SUCCESSS);
    return (0);
}

#else
# include <unistd.h>
# include <fcntl.h>
# include <cerrno>

void cmp_force_rng_open_failure(int32_t error_code);
void cmp_force_rng_read_failure(int32_t error_code);
void cmp_force_rng_read_eof(void);
void cmp_force_rng_close_failure(int32_t error_code);
void cmp_clear_force_rng_failures(void);

static int32_t g_force_rng_open_errno = 0;
static int32_t g_force_rng_read_errno = 0;
static int32_t g_force_rng_close_errno = 0;
static int32_t g_force_rng_read_zero = 0;

void cmp_force_rng_open_failure(int32_t error_code)
{
    g_force_rng_open_errno = error_code;
    return ;
}

void cmp_force_rng_read_failure(int32_t error_code)
{
    g_force_rng_read_errno = error_code;
    return ;
}

void cmp_force_rng_close_failure(int32_t error_code)
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

int32_t cmp_rng_secure_bytes(unsigned char *buffer, ft_size_t length)
{
    int32_t forced_open_errno = g_force_rng_open_errno;
    ssize_t bytes_read;
    int32_t file_descriptor;
    int32_t error_code;
    ft_size_t offset;

    g_force_rng_open_errno = 0;
    if (forced_open_errno != 0)
    {
        errno = forced_open_errno;
        error_code = ft_map_system_error(errno);
        cmp_set_last_error(error_code);
        return (-1);
    }
    file_descriptor = open("/dev/urandom", O_RDONLY);
    if (file_descriptor < 0)
    {
        error_code = ft_map_system_error(errno);
        cmp_set_last_error(error_code);
        return (-1);
    }
    offset = 0;
    while (offset < length)
    {
        int32_t forced_read_errno = g_force_rng_read_errno;

        if (forced_read_errno != 0)
        {
            g_force_rng_read_errno = 0;
            errno = forced_read_errno;
            error_code = ft_map_system_error(errno);
            int32_t stored_errno = errno;

            close(file_descriptor);
            errno = stored_errno;
            cmp_set_last_error(error_code);
            return (-1);
        }
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
            int32_t stored_errno = errno;

            error_code = ft_map_system_error(errno);
            close(file_descriptor);
            errno = stored_errno;
            cmp_set_last_error(error_code);
            return (-1);
        }
        if (bytes_read == 0)
        {
            int32_t close_result = close(file_descriptor);

            if (close_result < 0)
            {
                error_code = ft_map_system_error(errno);
                cmp_set_last_error(error_code);
                return (-1);
            }
            cmp_set_last_error(FT_ERR_IO);
            return (-1);
        }
        offset += static_cast<ft_size_t>(bytes_read);
    }
    if (close(file_descriptor) < 0)
    {
        error_code = ft_map_system_error(errno);
        cmp_set_last_error(error_code);
        return (-1);
    }
    int32_t forced_close_errno = g_force_rng_close_errno;

    g_force_rng_close_errno = 0;
    if (forced_close_errno != 0)
    {
        errno = forced_close_errno;
        error_code = ft_map_system_error(errno);
        cmp_set_last_error(error_code);
        return (-1);
    }
    cmp_set_last_error(FT_ERR_SUCCESSS);
    return (0);
}
#endif
