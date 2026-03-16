#include "../test_internal.hpp"
#include "../../Template/tuple.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_template_tuple_thread_safety_enable_disable)
{
    ft_tuple<int32_t, int32_t> tuple_instance;
    ft_bool lock_acquired;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, tuple_instance.initialize());
    tuple_instance.get<0>() = 5;
    tuple_instance.get<1>() = 7;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, tuple_instance.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, tuple_instance.is_thread_safe());

    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tuple_instance.lock(&lock_acquired));
    FT_ASSERT_EQ(FT_TRUE, lock_acquired);
    tuple_instance.unlock(lock_acquired);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, tuple_instance.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, tuple_instance.is_thread_safe());
    return (1);
}
