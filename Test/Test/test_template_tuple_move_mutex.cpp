#include "../test_internal.hpp"
#include "../../Template/tuple.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

using tuple_type = ft_tuple<int, int>;

FT_TEST(test_ft_tuple_move_constructor_rebuilds_mutex,
        "ft_tuple mutex can be enabled and protects values")
{
    tuple_type tuple_instance(5, 7);
    bool lock_acquired;

    FT_ASSERT_EQ(0, tuple_instance.enable_thread_safety());
    FT_ASSERT(tuple_instance.is_thread_safe());

    lock_acquired = false;
    FT_ASSERT_EQ(0, tuple_instance.lock(&lock_acquired));
    FT_ASSERT(lock_acquired);
    FT_ASSERT_EQ(5, tuple_instance.get<0>());
    FT_ASSERT_EQ(7, tuple_instance.get<1>());
    tuple_instance.unlock(lock_acquired);

    tuple_instance.disable_thread_safety();
    FT_ASSERT_EQ(false, tuple_instance.is_thread_safe());
    return (1);
}

FT_TEST(test_ft_tuple_move_assignment_rebuilds_mutex,
        "ft_tuple can reinitialize mutex after clear")
{
    ft_tuple<int, int> tuple_instance(1, 2);
    bool lock_acquired;

    FT_ASSERT_EQ(0, tuple_instance.enable_thread_safety());
    FT_ASSERT(tuple_instance.is_thread_safe());

    tuple_instance.reset();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tuple_type::last_operation_error());

    FT_ASSERT_EQ(0, tuple_instance.enable_thread_safety());
    lock_acquired = false;
    FT_ASSERT_EQ(0, tuple_instance.lock(&lock_acquired));
    FT_ASSERT(lock_acquired);
    tuple_instance.unlock(lock_acquired);
    tuple_instance.disable_thread_safety();
    return (1);
}
