#include "../test_internal.hpp"
#include "../../Encryption/encryption.hpp"
#include "../../CMA/CMA.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include <cstring>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_encryption_key_random)
{
    const char *first_key;
    const char *second_key;

    first_key = be_get_encryption_key();
    FT_ASSERT(first_key != ft_nullptr);
    second_key = be_get_encryption_key();
    FT_ASSERT(second_key != ft_nullptr);
    FT_ASSERT(std::strcmp(first_key, second_key) != 0);
    cma_free(const_cast<char *>(first_key));
    cma_free(const_cast<char *>(second_key));
    return (1);
}
