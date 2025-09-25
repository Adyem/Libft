#include "ssl_wrapper.hpp"
#include <climits>

static ssize_t ssl_write_platform(SSL *ssl, const void *buf, size_t len)
{
    int write_length;
    int ret;

    if (len > static_cast<size_t>(INT_MAX))
        return (-1);
    write_length = static_cast<int>(len);
    ret = SSL_write(ssl, buf, write_length);
    if (ret <= 0)
        return (-1);
    return (ret);
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
            return (-1);
        read_length = static_cast<int>(len);
        ret = SSL_read(ssl, buf, read_length);
        if (ret <= 0)
            return (-1);
        return (ret);
    }
}
