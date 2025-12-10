#include <openssl/evp.h>
#include "../Errno/errno.hpp"
#include "encryption_sha3.hpp"

static void sha3_hash_internal(const EVP_MD *algorithm, const void *data,
    size_t length, unsigned char *digest, size_t expected_length)
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
    if (static_cast<size_t>(output_length) != expected_length)
    {
        ft_errno = FT_ERR_INTERNAL;
        return ;
    }
    ft_errno = FT_ER_SUCCESSS;
    return ;
}

void sha3_256_hash(const void *data, size_t length, unsigned char *digest)
{
    sha3_hash_internal(EVP_sha3_256(), data, length, digest, 32);
    return ;
}

void sha3_512_hash(const void *data, size_t length, unsigned char *digest)
{
    sha3_hash_internal(EVP_sha3_512(), data, length, digest, 64);
    return ;
}
