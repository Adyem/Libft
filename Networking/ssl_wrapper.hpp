#ifndef NETWORKING_SSL_WRAPPER_HPP
#define NETWORKING_SSL_WRAPPER_HPP

#include <openssl/ssl.h>
#include <cstddef>

#ifdef _WIN32
# include <basetsd.h>
typedef SSIZE_T ssize_t;
#else
# include <sys/types.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

ssize_t nw_ssl_write(SSL *ssl, const void *buf, size_t len);
ssize_t nw_ssl_read(SSL *ssl, void *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif
