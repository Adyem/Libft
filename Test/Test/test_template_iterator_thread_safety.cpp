#include "../test_internal.hpp"
#include "../../Template/iterator.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_iterator_thread_safety_resets_errno,
        "Iterator installs optional mutex guards and resets errno to success")
{
    int values[2] = {1, 2};
    Iterator<int> iterator(values);
    bool lock_acquired;

    FT_ASSERT_EQ(false, iterator.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, iterator.enable_thread_safety());
    FT_ASSERT_EQ(true, iterator.is_thread_safe_enabled());
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    lock_acquired = false;
    FT_ASSERT_EQ(0, iterator.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    iterator.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    iterator.disable_thread_safety();
    FT_ASSERT_EQ(false, iterator.is_thread_safe_enabled());
    return (1);
}

FT_TEST(test_iterator_copy_and_move_preserve_thread_safety,
        "Iterator copies and moves propagate thread safety state")
{
    int values[3] = {5, 10, 15};
    Iterator<int> original(values);

    FT_ASSERT_EQ(0, original.enable_thread_safety());
    FT_ASSERT_EQ(true, original.is_thread_safe_enabled());
    FT_ASSERT_EQ(5, *original);

    Iterator<int> copied(original);
    FT_ASSERT_EQ(true, copied.is_thread_safe_enabled());
    FT_ASSERT_EQ(5, *copied);

    Iterator<int> assigned(values + 1);
    assigned = original;
    FT_ASSERT_EQ(true, assigned.is_thread_safe_enabled());
    FT_ASSERT_EQ(5, *assigned);

    Iterator<int> moved(ft_move(original));
    FT_ASSERT_EQ(true, moved.is_thread_safe_enabled());
    FT_ASSERT_EQ(5, *moved);

    Iterator<int> move_assigned(values + 2);
    FT_ASSERT_EQ(0, move_assigned.enable_thread_safety());
    move_assigned = ft_move(copied);
    FT_ASSERT_EQ(true, move_assigned.is_thread_safe_enabled());
    FT_ASSERT_EQ(5, *move_assigned);

    return (1);
}
