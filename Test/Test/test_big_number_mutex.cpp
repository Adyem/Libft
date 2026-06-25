#include "../test_internal.hpp"
#include "../../Modules/CPP_class/class_big_number.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
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

FT_TEST(test_big_number_move_constructor_preserves_thread_safety_and_value)
{
    ft_big_number source_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize());
    source_value.assign("123456");
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.enable_thread_safety());

    ft_big_number moved_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_value.initialize(static_cast<ft_big_number &&>(source_value)));

    FT_ASSERT_EQ(FT_CLASS_STATE_DESTROYED, source_value._initialised_state);
    FT_ASSERT_EQ(ft_nullptr, source_value._mutex);
    FT_ASSERT_EQ(FT_TRUE, moved_value.is_thread_safe());
    FT_ASSERT(moved_value._mutex != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp(moved_value.c_str(), "123456"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_value.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_value.destroy());
    return (1);
}

FT_TEST(test_big_number_move_method_preserves_thread_safety_and_value)
{
    ft_big_number source_value;
    ft_big_number destination_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize());
    source_value.assign("789012");
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.move(source_value));
    FT_ASSERT_EQ(FT_CLASS_STATE_DESTROYED, source_value._initialised_state);
    FT_ASSERT_EQ(ft_nullptr, source_value._mutex);
    FT_ASSERT_EQ(FT_TRUE, destination_value.is_thread_safe());
    FT_ASSERT(destination_value._mutex != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp(destination_value.c_str(), "789012"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    return (1);
}
