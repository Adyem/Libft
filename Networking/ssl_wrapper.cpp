#include "ssl_wrapper.hpp"

ssize_t nw_ssl_write(SSL *ssl, const void *buf, size_t len)
{
    int ret = SSL_write(ssl, buf, static_cast<int>(len));
    if (ret <= 0)
        return (-1);
    return (ret);
}

ssize_t nw_ssl_read(SSL *ssl, void *buf, size_t len)
{
    int ret = SSL_read(ssl, buf, static_cast<int>(len));
    if (ret <= 0)
        return (-1);
    return (ret);
}
