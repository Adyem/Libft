#include "../test_internal.hpp"
#include "../../Encryption/basic_encryption.hpp"
#include "../../CMA/CMA.hpp"
#include <cstring>

#ifndef LIBFT_TEST_BUILD
#endif

int test_encryption_key_random(void)
{
    const char *first_key = be_getEncryptionKey();
    if (!first_key)
        return (0);
    const char *second_key = be_getEncryptionKey();
    if (!second_key)
    {
        cma_free(const_cast<char *>(first_key));
        return (0);
    }
    int result = 1;
    if (std::strcmp(first_key, second_key) == 0)
        result = 0;
    cma_free(const_cast<char *>(first_key));
    cma_free(const_cast<char *>(second_key));
    return (result);
}

