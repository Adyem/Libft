#include "ssl_wrapper.hpp"
#include <climits>
#include "../Errno/errno.hpp"

#if NETWORKING_HAS_OPENSSL

static ssize_t ssl_translate_result(SSL *ssl, int32_t result, int32_t *error_code)
{
    int32_t ssl_error;

    if (result > 0)
    {
        if (error_code)
            *error_code = FT_ERR_SUCCESS;
        return (result);
    }
    ssl_error = SSL_get_error(ssl, result);
    if (ssl_error == SSL_ERROR_WANT_READ)
    {
        if (error_code)
            *error_code = FT_ERR_SSL_WANT_READ;
        return (0);
    }
    if (ssl_error == SSL_ERROR_WANT_WRITE)
    {
        if (error_code)
            *error_code = FT_ERR_SSL_WANT_WRITE;
        return (0);
    }
    if (ssl_error == SSL_ERROR_ZERO_RETURN)
    {
        if (error_code)
            *error_code = FT_ERR_SSL_ZERO_RETURN;
        return (0);
    }
    if (ssl_error == SSL_ERROR_SYSCALL)
    {
        if (error_code)
            *error_code = FT_ERR_SSL_SYSCALL_ERROR;
        return (-1);
    }
    if (error_code)
        *error_code = FT_ERR_IO;
    return (-1);
}

static ssize_t ssl_report(int32_t error_code, ssize_t result)
{
    (void)(error_code);
    return (result);
}

static ssize_t ssl_write_platform(SSL *ssl, const void *buf, ft_size_t len)
{
    int32_t write_length;
    int32_t ret;
    int32_t error_code;

    if (len > static_cast<ft_size_t>(INT_MAX))
    {
        return (ssl_report(FT_ERR_INVALID_ARGUMENT, -1));
    }
    write_length = static_cast<int32_t>(len);
    ret = SSL_write(ssl, buf, write_length);
    return (ssl_report(error_code, ssl_translate_result(ssl, ret, &error_code)));
}

extern "C"
{
    ssize_t nw_ssl_write(SSL *ssl, const void *buf, ft_size_t len)
    {
        ssize_t result;

        result = ssl_write_platform(ssl, buf, len);
        return (result);
    }

    ssize_t nw_ssl_read(SSL *ssl, void *buf, ft_size_t len)
    {
        int32_t read_length;
        int32_t ret;
        int32_t error_code;

        if (len > static_cast<ft_size_t>(INT_MAX))
        {
            return (ssl_report(FT_ERR_INVALID_ARGUMENT, -1));
        }
        read_length = static_cast<int32_t>(len);
        ret = SSL_read(ssl, buf, read_length);
        return (ssl_report(error_code, ssl_translate_result(ssl, ret, &error_code)));
    }
}

#endif
