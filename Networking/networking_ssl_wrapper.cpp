#include "ssl_wrapper.hpp"
#include <climits>
#include "../Errno/errno.hpp"

static ssize_t ssl_translate_result(SSL *ssl, int result)
{
    int ssl_error;

    if (result > 0)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (result);
    }
    ssl_error = SSL_get_error(ssl, result);
    if (ssl_error == SSL_ERROR_WANT_READ)
    {
        ft_errno = FT_ERR_SSL_WANT_READ;
        return (0);
    }
    if (ssl_error == SSL_ERROR_WANT_WRITE)
    {
        ft_errno = FT_ERR_SSL_WANT_WRITE;
        return (0);
    }
    if (ssl_error == SSL_ERROR_ZERO_RETURN)
    {
        ft_errno = FT_ERR_SSL_ZERO_RETURN;
        return (0);
    }
    if (ssl_error == SSL_ERROR_SYSCALL)
    {
        ft_errno = FT_ERR_SSL_SYSCALL_ERROR;
        return (-1);
    }
    ft_errno = FT_ERR_IO;
    return (-1);
}

static ssize_t ssl_write_platform(SSL *ssl, const void *buf, size_t len)
{
    int write_length;
    int ret;

    if (len > static_cast<size_t>(INT_MAX))
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    write_length = static_cast<int>(len);
    ret = SSL_write(ssl, buf, write_length);
    return (ssl_translate_result(ssl, ret));
}

extern "C"
{
    ssize_t nw_ssl_write(SSL *ssl, const void *buf, size_t len)
    {
        return (ssl_write_platform(ssl, buf, len));
    }

    ssize_t nw_ssl_read(SSL *ssl, void *buf, size_t len)
    {
        int read_length;
        int ret;

        if (len > static_cast<size_t>(INT_MAX))
        {
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (-1);
        }
        read_length = static_cast<int>(len);
        ret = SSL_read(ssl, buf, read_length);
        return (ssl_translate_result(ssl, ret));
    }
}
