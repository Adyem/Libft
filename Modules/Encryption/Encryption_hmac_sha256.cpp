#include "../Networking/openssl_support.hpp"

#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#if NETWORKING_HAS_OPENSSL
#include <openssl/evp.h>
#include <openssl/core_names.h>
#include <openssl/params.h>
#include "../Errno/errno.hpp"
#include "../Basic/class_nullptr.hpp"
#include "encryption.hpp"

static void hmac_sha256_stream_reset_state(hmac_sha256_stream &stream)
{
    stream.mac_handle = ft_nullptr;
    stream.context_handle = ft_nullptr;
    stream.initialised = FT_FALSE;
    stream.finished = FT_FALSE;
    return ;
}

static void hmac_sha256_stream_release(hmac_sha256_stream &stream)
{
    if (stream.context_handle != ft_nullptr)
        EVP_MAC_CTX_free(stream.context_handle);
    stream.context_handle = ft_nullptr;
    if (stream.mac_handle != ft_nullptr)
        EVP_MAC_free(stream.mac_handle);
    stream.mac_handle = ft_nullptr;
    return ;
}

int32_t hmac_sha256_stream_init(hmac_sha256_stream &stream,
    const uint8_t *key_buffer, ft_size_t key_length)
{
    OSSL_PARAM  parameters[2];
    char        digest_name[7];

    if (key_length > 0 && key_buffer == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    hmac_sha256_stream_reset_state(stream);
    digest_name[0] = 'S';
    digest_name[1] = 'H';
    digest_name[2] = 'A';
    digest_name[3] = '2';
    digest_name[4] = '5';
    digest_name[5] = '6';
    digest_name[6] = '\0';
    stream.mac_handle = EVP_MAC_fetch(ft_nullptr, "HMAC", ft_nullptr);
    if (stream.mac_handle == ft_nullptr)
        return (FT_ERR_INTERNAL);
    stream.context_handle = EVP_MAC_CTX_new(stream.mac_handle);
    if (stream.context_handle == ft_nullptr)
    {
        hmac_sha256_stream_release(stream);
        return (FT_ERR_NO_MEMORY);
    }
    parameters[0] = OSSL_PARAM_construct_utf8_string(OSSL_MAC_PARAM_DIGEST,
        digest_name, 0);
    parameters[1] = OSSL_PARAM_construct_end();
    if (EVP_MAC_init(stream.context_handle, key_buffer, key_length,
            parameters) != 1)
    {
        hmac_sha256_stream_release(stream);
        return (FT_ERR_INITIALIZATION_FAILED);
    }
    stream.initialised = FT_TRUE;
    stream.finished = FT_FALSE;
    return (FT_ERR_SUCCESS);
}

int32_t hmac_sha256_stream_update(hmac_sha256_stream &stream, const void *data,
    ft_size_t length)
{
    if (stream.initialised == FT_FALSE
        || stream.context_handle == ft_nullptr)
        return (FT_ERR_INVALID_STATE);
    if (stream.finished != FT_FALSE)
        return (FT_ERR_INVALID_STATE);
    if (length == 0)
        return (FT_ERR_SUCCESS);
    if (data == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (EVP_MAC_update(stream.context_handle,
            static_cast<const uint8_t *>(data), length) != 1)
        return (FT_ERR_INTERNAL);
    return (FT_ERR_SUCCESS);
}

int32_t hmac_sha256_stream_final(hmac_sha256_stream &stream, uint8_t *digest,
    ft_size_t digest_length)
{
    ft_size_t output_length;

    if (stream.initialised == FT_FALSE
        || stream.context_handle == ft_nullptr)
        return (FT_ERR_INVALID_STATE);
    if (stream.finished != FT_FALSE)
        return (FT_ERR_INVALID_STATE);
    if (digest == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (digest_length < 32)
        return (FT_ERR_INVALID_ARGUMENT);
    output_length = digest_length;
    if (EVP_MAC_final(stream.context_handle, digest, &output_length,
            digest_length) != 1)
        return (FT_ERR_INTERNAL);
    if (output_length != 32)
        return (FT_ERR_INTERNAL);
    hmac_sha256_stream_release(stream);
    stream.initialised = FT_FALSE;
    stream.finished = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

void hmac_sha256_stream_cleanup(hmac_sha256_stream &stream)
{
    hmac_sha256_stream_release(stream);
    stream.initialised = FT_FALSE;
    stream.finished = FT_TRUE;
    return ;
}

void hmac_sha256(const uint8_t *key_buffer, ft_size_t key_length,
    const void *data_buffer, ft_size_t data_length, uint8_t *digest_buffer)
{
    hmac_sha256_stream    stream;

    if (hmac_sha256_stream_init(stream, key_buffer, key_length)
        != FT_ERR_SUCCESS)
        return ;
    if (hmac_sha256_stream_update(stream, data_buffer, data_length)
        != FT_ERR_SUCCESS)
    {
        hmac_sha256_stream_cleanup(stream);
        return ;
    }
    if (hmac_sha256_stream_final(stream, digest_buffer, 32) != FT_ERR_SUCCESS)
    {
        hmac_sha256_stream_cleanup(stream);
        return ;
    }
    hmac_sha256_stream_cleanup(stream);
    return ;
}

#endif
