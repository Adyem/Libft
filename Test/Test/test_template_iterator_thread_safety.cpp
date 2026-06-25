#include "../test_internal.hpp"
#include "../../Modules/Template/iterator.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_template_iterator_thread_safety_enable_disable)
{
    int32_t values[2] = {1, 2};
    Iterator<int32_t> iterator;
    ft_bool lock_acquired;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, iterator.initialize(values));
    FT_ASSERT_EQ(FT_FALSE, iterator.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, iterator.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, iterator.is_thread_safe());

    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, iterator.lock(&lock_acquired));
    FT_ASSERT_EQ(FT_TRUE, lock_acquired);
    iterator.unlock(lock_acquired);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, iterator.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, iterator.is_thread_safe());
    return (1);
}
