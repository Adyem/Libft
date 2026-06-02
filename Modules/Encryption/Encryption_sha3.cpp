#include "../Networking/openssl_support.hpp"

#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#if NETWORKING_HAS_OPENSSL
#include <openssl/evp.h>
#include "../Errno/errno.hpp"
#include "../Basic/class_nullptr.hpp"
#include "encryption.hpp"

static int32_t sha3_hash_internal(const EVP_MD *algorithm, const void *data,
    ft_size_t length, uint8_t *digest, ft_size_t expected_length)
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
    if (static_cast<ft_size_t>(output_length) != expected_length)
    {
        return (FT_ERR_INTERNAL);
    }
    return (FT_ERR_SUCCESS);
}

int32_t sha3_256_hash(const void *data, ft_size_t length, uint8_t *digest)
{
    return (sha3_hash_internal(EVP_sha3_256(), data, length, digest, 32));
}

int32_t sha3_512_hash(const void *data, ft_size_t length, uint8_t *digest)
{
    return (sha3_hash_internal(EVP_sha3_512(), data, length, digest, 64));
}

#endif
