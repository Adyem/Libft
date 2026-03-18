#include "../test_internal.hpp"
#include "../../CPP_class/class_big_number.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_big_number_mutex_enable_disable_cycle)
{
    ft_big_number number;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, number.initialize());
    FT_ASSERT_EQ(FT_FALSE, number.is_thread_safe());
    FT_ASSERT_EQ(ft_nullptr, number._mutex);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, number.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, number.is_thread_safe());
    FT_ASSERT(number._mutex != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, number.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, number.is_thread_safe());
    FT_ASSERT_EQ(ft_nullptr, number._mutex);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, number.destroy());
    return (1);
}

FT_TEST(test_big_number_mutex_enable_is_retryable)
{
    ft_big_number number;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, number.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, number.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, number.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, number.is_thread_safe());
    FT_ASSERT(number._mutex != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, number.destroy());
    FT_ASSERT_EQ(FT_CLASS_STATE_DESTROYED, number._initialised_state);
    FT_ASSERT_EQ(ft_nullptr, number._mutex);
    return (1);
}

FT_TEST(test_big_number_mutex_destroy_disables_thread_safety)
{
    ft_big_number number;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, number.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, number.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, number.is_thread_safe());
    FT_ASSERT(number._mutex != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, number.destroy());
    FT_ASSERT_EQ(FT_CLASS_STATE_DESTROYED, number._initialised_state);
    FT_ASSERT_EQ(ft_nullptr, number._mutex);
    return (1);
}
