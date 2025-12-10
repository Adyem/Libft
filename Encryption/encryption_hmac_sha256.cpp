#include <openssl/evp.h>
#include <openssl/core_names.h>
#include <openssl/params.h>
#include "../Errno/errno.hpp"
#include "encryption_hmac_sha256.hpp"

static void hmac_sha256_stream_reset_state(hmac_sha256_stream &stream)
{
    stream.mac = NULL;
    stream.context = NULL;
    stream.initialized = false;
    stream.finished = false;
    return ;
}

static void hmac_sha256_stream_release(hmac_sha256_stream &stream)
{
    if (stream.context != NULL)
        EVP_MAC_CTX_free(stream.context);
    stream.context = NULL;
    if (stream.mac != NULL)
        EVP_MAC_free(stream.mac);
    stream.mac = NULL;
    return ;
}

int hmac_sha256_stream_init(hmac_sha256_stream &stream, const unsigned char *key,
    size_t key_length)
{
    OSSL_PARAM  parameters[2];
    char        digest_name[7];

    if (key_length > 0 && key == NULL)
    {
        ft_errno = FT_ERR_INVALID_POINTER;
        return (-1);
    }
    hmac_sha256_stream_reset_state(stream);
    digest_name[0] = 'S';
    digest_name[1] = 'H';
    digest_name[2] = 'A';
    digest_name[3] = '2';
    digest_name[4] = '5';
    digest_name[5] = '6';
    digest_name[6] = '\0';
    stream.mac = EVP_MAC_fetch(NULL, "HMAC", NULL);
    if (stream.mac == NULL)
    {
        ft_errno = FT_ERR_INITIALIZATION_FAILED;
        return (-1);
    }
    stream.context = EVP_MAC_CTX_new(stream.mac);
    if (stream.context == NULL)
    {
        hmac_sha256_stream_release(stream);
        ft_errno = FT_ERR_NO_MEMORY;
        return (-1);
    }
    parameters[0] = OSSL_PARAM_construct_utf8_string(OSSL_MAC_PARAM_DIGEST,
        digest_name, 0);
    parameters[1] = OSSL_PARAM_construct_end();
    if (EVP_MAC_init(stream.context, key, key_length, parameters) != 1)
    {
        hmac_sha256_stream_release(stream);
        ft_errno = FT_ERR_INITIALIZATION_FAILED;
        return (-1);
    }
    stream.initialized = true;
    stream.finished = false;
    ft_errno = FT_ER_SUCCESSS;
    return (0);
}

int hmac_sha256_stream_update(hmac_sha256_stream &stream, const void *data,
    size_t length)
{
    if (stream.initialized == false || stream.context == NULL)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (-1);
    }
    if (stream.finished != false)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (-1);
    }
    if (length == 0)
    {
        ft_errno = FT_ER_SUCCESSS;
        return (0);
    }
    if (data == NULL)
    {
        ft_errno = FT_ERR_INVALID_POINTER;
        return (-1);
    }
    if (EVP_MAC_update(stream.context,
            static_cast<const unsigned char *>(data), length) != 1)
    {
        ft_errno = FT_ERR_INTERNAL;
        return (-1);
    }
    ft_errno = FT_ER_SUCCESSS;
    return (0);
}

int hmac_sha256_stream_final(hmac_sha256_stream &stream, unsigned char *digest,
    size_t digest_length)
{
    size_t output_length;

    if (stream.initialized == false || stream.context == NULL)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (-1);
    }
    if (stream.finished != false)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (-1);
    }
    if (digest == NULL)
    {
        ft_errno = FT_ERR_INVALID_POINTER;
        return (-1);
    }
    if (digest_length < 32)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    output_length = digest_length;
    if (EVP_MAC_final(stream.context, digest, &output_length, digest_length) != 1)
    {
        ft_errno = FT_ERR_INTERNAL;
        return (-1);
    }
    if (output_length != 32)
    {
        ft_errno = FT_ERR_INTERNAL;
        return (-1);
    }
    hmac_sha256_stream_release(stream);
    stream.initialized = false;
    stream.finished = true;
    ft_errno = FT_ER_SUCCESSS;
    return (0);
}

void hmac_sha256_stream_cleanup(hmac_sha256_stream &stream)
{
    hmac_sha256_stream_release(stream);
    stream.initialized = false;
    stream.finished = true;
    return ;
}

void hmac_sha256(const unsigned char *key, size_t key_len, const void *data,
    size_t len, unsigned char *digest)
{
    hmac_sha256_stream    stream;

    if (hmac_sha256_stream_init(stream, key, key_len) != 0)
        return ;
    if (hmac_sha256_stream_update(stream, data, len) != 0)
    {
        hmac_sha256_stream_cleanup(stream);
        return ;
    }
    if (hmac_sha256_stream_final(stream, digest, 32) != 0)
    {
        hmac_sha256_stream_cleanup(stream);
        return ;
    }
    hmac_sha256_stream_cleanup(stream);
    ft_errno = FT_ER_SUCCESSS;
    return ;
}
