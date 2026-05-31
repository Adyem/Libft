#include "../Networking/openssl_support.hpp"

#if NETWORKING_HAS_OPENSSL
#include <openssl/evp.h>
#include "../Errno/errno.hpp"
#include "../Basic/class_nullptr.hpp"
#include "encryption.hpp"

static int32_t blake2_hash_internal(const EVP_MD *algorithm, const void *data,
    ft_size_t length, uint8_t *digest, ft_size_t digest_length,
    ft_size_t max_length)
{
    EVP_MD_CTX  *context;
    uint32_t output_length;

    if (digest == ft_nullptr)
    {
        return (FT_ERR_INVALID_POINTER);
    }
    if (length > 0 && data == ft_nullptr)
    {
        return (FT_ERR_INVALID_POINTER);
    }
    if (digest_length == 0 || digest_length > max_length)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (algorithm == ft_nullptr)
    {
        return (FT_ERR_INVALID_OPERATION);
    }
    context = EVP_MD_CTX_new();
    if (context == ft_nullptr)
    {
        return (FT_ERR_NO_MEMORY);
    }
    if (EVP_DigestInit_ex(context, algorithm, ft_nullptr) != 1)
    {
        EVP_MD_CTX_free(context);
        return (FT_ERR_INITIALIZATION_FAILED);
    }
    if (digest_length != max_length)
    {
        EVP_MD_CTX_free(context);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (length > 0)
    {
        if (EVP_DigestUpdate(context, data, length) != 1)
        {
            EVP_MD_CTX_free(context);
            return (FT_ERR_INTERNAL);
        }
    }
    output_length = 0;
    if (EVP_DigestFinal_ex(context, digest, &output_length) != 1)
    {
        EVP_MD_CTX_free(context);
        return (FT_ERR_INTERNAL);
    }
    EVP_MD_CTX_free(context);
    if (static_cast<ft_size_t>(output_length) != digest_length)
    {
        return (FT_ERR_INTERNAL);
    }
    return (FT_ERR_SUCCESS);
}

int32_t blake2b_hash(const void *data, ft_size_t length, uint8_t *digest,
    ft_size_t digest_length)
{
    return (blake2_hash_internal(EVP_blake2b512(), data, length, digest,
            digest_length, 64));
}

int32_t blake2s_hash(const void *data, ft_size_t length, uint8_t *digest,
    ft_size_t digest_length)
{
    return (blake2_hash_internal(EVP_blake2s256(), data, length, digest,
            digest_length, 32));
}

#endif
