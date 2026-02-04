#ifndef NETWORKING_OPENSSL_SUPPORT_HPP
#define NETWORKING_OPENSSL_SUPPORT_HPP

#if !defined(NETWORKING_HAS_OPENSSL)
# if defined(__has_include)
#  if __has_include(<openssl/ssl.h>)
#   include <openssl/ssl.h>
#   define NETWORKING_HAS_OPENSSL 1
#  else
#   define NETWORKING_HAS_OPENSSL 0
#  endif
# else
#  define NETWORKING_HAS_OPENSSL 0
# endif
#endif

#endif
