#include "../Networking/openssl_support.hpp"

#if NETWORKING_HAS_OPENSSL
#include <openssl/evp.h>
#include "../Errno/errno.hpp"
#include "encryption_sha3.hpp"

static int sha3_hash_internal(const EVP_MD *algorithm, const void *data,
    size_t length, unsigned char *digest, size_t expected_length)
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
    if (static_cast<size_t>(output_length) != expected_length)
    {
        return (FT_ERR_INTERNAL);
    }
    return (FT_ERR_SUCCESS);
}

void sha3_256_hash(const void *data, size_t length, unsigned char *digest)
{
    int error_code;

    error_code = sha3_hash_internal(EVP_sha3_256(), data, length, digest, 32);
    ft_global_error_stack_push(error_code);
    return ;
}

void sha3_512_hash(const void *data, size_t length, unsigned char *digest)
{
    int error_code;

    error_code = sha3_hash_internal(EVP_sha3_512(), data, length, digest, 64);
    ft_global_error_stack_push(error_code);
    return ;
}

#endif
