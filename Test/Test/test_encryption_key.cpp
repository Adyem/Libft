#include "../test_internal.hpp"
#include "../../Modules/Encryption/encryption.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <cstring>

#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
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
