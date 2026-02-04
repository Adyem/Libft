#include "../Networking/openssl_support.hpp"

#if NETWORKING_HAS_OPENSSL
#include <openssl/evp.h>
#include "../Errno/errno.hpp"
#include "encryption_blake2.hpp"

static int blake2_hash_internal(const EVP_MD *algorithm, const void *data,
    size_t length, unsigned char *digest, size_t digest_length,
    size_t max_length)
{
    EVP_MD_CTX  *context;
    unsigned int output_length;

    if (digest == NULL)
    {
        return (FT_ERR_INVALID_POINTER);
    }
    if (length > 0 && data == NULL)
    {
        return (FT_ERR_INVALID_POINTER);
    }
    if (digest_length == 0 || digest_length > max_length)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (algorithm == NULL)
    {
        return (FT_ERR_INVALID_OPERATION);
    }
    context = EVP_MD_CTX_new();
    if (context == NULL)
    {
        return (FT_ERR_NO_MEMORY);
    }
    if (EVP_DigestInit_ex(context, algorithm, NULL) != 1)
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
    if (static_cast<size_t>(output_length) != digest_length)
    {
        return (FT_ERR_INTERNAL);
    }
    return (FT_ERR_SUCCESSS);
}

void blake2b_hash(const void *data, size_t length, unsigned char *digest,
    size_t digest_length)
{
    int error_code;

    error_code = blake2_hash_internal(EVP_blake2b512(), data, length, digest,
            digest_length, 64);
    ft_global_error_stack_push(error_code);
    return ;
}

void blake2s_hash(const void *data, size_t length, unsigned char *digest,
    size_t digest_length)
{
    int error_code;

    error_code = blake2_hash_internal(EVP_blake2s256(), data, length, digest,
            digest_length, 32);
    ft_global_error_stack_push(error_code);
    return ;
}

#endif
