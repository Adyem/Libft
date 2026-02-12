#include "../test_internal.hpp"
#include "../../Template/unique_ptr.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_uniqueptr_move_constructor_rebuilds_mutex,
        "ft_uniqueptr move constructor reinitializes thread safety and transfers ownership")
{
    ft_uniqueptr<int> source_pointer(new int(42));
    ft_uniqueptr<int> moved_pointer;
    bool source_lock_acquired;
    bool moved_lock_acquired;

    FT_ASSERT_EQ(0, source_pointer.enable_thread_safety());
    FT_ASSERT(source_pointer.is_thread_safe());

    moved_pointer = ft_uniqueptr<int>(ft_move(source_pointer));

    FT_ASSERT(moved_pointer.is_thread_safe());
    FT_ASSERT_EQ(false, source_pointer.is_thread_safe());
    moved_lock_acquired = false;
    FT_ASSERT_EQ(0, moved_pointer.lock(&moved_lock_acquired));
    FT_ASSERT(moved_lock_acquired);
    FT_ASSERT_EQ(42, *moved_pointer);
    moved_pointer.unlock(moved_lock_acquired);
    source_lock_acquired = false;
    FT_ASSERT_EQ(0, source_pointer.lock(&source_lock_acquired));
    FT_ASSERT_EQ(false, source_lock_acquired);
    FT_ASSERT_EQ(0, source_pointer.enable_thread_safety());
    FT_ASSERT(source_pointer.is_thread_safe());
    source_pointer.reset(new int(7));
    source_lock_acquired = false;
    FT_ASSERT_EQ(0, source_pointer.lock(&source_lock_acquired));
    FT_ASSERT(source_lock_acquired);
    FT_ASSERT_EQ(7, *source_pointer);
    source_pointer.unlock(source_lock_acquired);
    return (1);
}

FT_TEST(test_ft_uniqueptr_move_assignment_rebuilds_mutex,
        "ft_uniqueptr move assignment recreates mutex and keeps destination usable")
{
    ft_uniqueptr<int> destination_pointer(new int(1));
    ft_uniqueptr<int> source_pointer(new int(9));
    bool destination_lock_acquired;
    bool source_lock_acquired;

    FT_ASSERT_EQ(0, destination_pointer.enable_thread_safety());
    FT_ASSERT_EQ(0, source_pointer.enable_thread_safety());
    FT_ASSERT(destination_pointer.is_thread_safe());
    FT_ASSERT(source_pointer.is_thread_safe());

    destination_lock_acquired = false;
    FT_ASSERT_EQ(0, destination_pointer.lock(&destination_lock_acquired));
    FT_ASSERT(destination_lock_acquired);
    destination_pointer.unlock(destination_lock_acquired);

    destination_pointer = ft_move(source_pointer);

    FT_ASSERT(destination_pointer.is_thread_safe());
    FT_ASSERT_EQ(false, source_pointer.is_thread_safe());
    FT_ASSERT_EQ(9, *destination_pointer);
    destination_lock_acquired = false;
    FT_ASSERT_EQ(0, destination_pointer.lock(&destination_lock_acquired));
    FT_ASSERT(destination_lock_acquired);
    FT_ASSERT_EQ(9, *destination_pointer);
    destination_pointer.unlock(destination_lock_acquired);
    FT_ASSERT_EQ(0, source_pointer.enable_thread_safety());
    FT_ASSERT(source_pointer.is_thread_safe());
    source_pointer.reset(new int(5));
    source_lock_acquired = false;
    FT_ASSERT_EQ(0, source_pointer.lock(&source_lock_acquired));
    FT_ASSERT(source_lock_acquired);
    FT_ASSERT_EQ(5, *source_pointer);
    source_pointer.unlock(source_lock_acquired);
    return (1);
}
