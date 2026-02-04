#ifndef ENCRYPTION_HMAC_SHA256_HPP
#define ENCRYPTION_HMAC_SHA256_HPP

#include <stddef.h>
#include "../Networking/openssl_support.hpp"

#if NETWORKING_HAS_OPENSSL
#include <openssl/evp.h>

struct hmac_sha256_stream
{
    EVP_MAC     *mac;
    EVP_MAC_CTX *context;
    bool        initialized;
    bool        finished;
};

int hmac_sha256_stream_init(hmac_sha256_stream &stream, const unsigned char *key,
    size_t key_length);
int hmac_sha256_stream_update(hmac_sha256_stream &stream, const void *data,
    size_t length);
int hmac_sha256_stream_final(hmac_sha256_stream &stream, unsigned char *digest,
    size_t digest_length);
void hmac_sha256_stream_cleanup(hmac_sha256_stream &stream);

void hmac_sha256(const unsigned char *key, size_t key_len, const void *data, size_t len, unsigned char *digest);

#endif

#endif
