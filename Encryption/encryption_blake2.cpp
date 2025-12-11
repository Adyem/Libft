#include <openssl/evp.h>
#include "../Errno/errno.hpp"
#include "encryption_blake2.hpp"

static void blake2_hash_internal(const EVP_MD *algorithm, const void *data,
    size_t length, unsigned char *digest, size_t digest_length,
    size_t max_length)
{
    EVP_MD_CTX  *context;
    unsigned int output_length;

    if (digest == NULL)
    {
        ft_errno = FT_ERR_INVALID_POINTER;
        return ;
    }
    if (length > 0 && data == NULL)
    {
        ft_errno = FT_ERR_INVALID_POINTER;
        return ;
    }
    if (digest_length == 0 || digest_length > max_length)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    if (algorithm == NULL)
    {
        ft_errno = FT_ERR_INVALID_OPERATION;
        return ;
    }
    context = EVP_MD_CTX_new();
    if (context == NULL)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return ;
    }
    if (EVP_DigestInit_ex(context, algorithm, NULL) != 1)
    {
        EVP_MD_CTX_free(context);
        ft_errno = FT_ERR_INITIALIZATION_FAILED;
        return ;
    }
    if (digest_length != max_length)
    {
        EVP_MD_CTX_free(context);
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    if (length > 0)
    {
        if (EVP_DigestUpdate(context, data, length) != 1)
        {
            EVP_MD_CTX_free(context);
            ft_errno = FT_ERR_INTERNAL;
            return ;
        }
    }
    output_length = 0;
    if (EVP_DigestFinal_ex(context, digest, &output_length) != 1)
    {
        EVP_MD_CTX_free(context);
        ft_errno = FT_ERR_INTERNAL;
        return ;
    }
    EVP_MD_CTX_free(context);
    if (static_cast<size_t>(output_length) != digest_length)
    {
        ft_errno = FT_ERR_INTERNAL;
        return ;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}

void blake2b_hash(const void *data, size_t length, unsigned char *digest,
    size_t digest_length)
{
    blake2_hash_internal(EVP_blake2b512(), data, length, digest,
        digest_length, 64);
    return ;
}

void blake2s_hash(const void *data, size_t length, unsigned char *digest,
    size_t digest_length)
{
    blake2_hash_internal(EVP_blake2s256(), data, length, digest,
        digest_length, 32);
    return ;
}
