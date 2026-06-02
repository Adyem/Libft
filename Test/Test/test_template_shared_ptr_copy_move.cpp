#include "../test_internal.hpp"
#include "../../Modules/Template/shared_ptr.hpp"
#include "../../Modules/Template/move.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_sharedptr_copy_constructor_rebuilds_mutex)
{
    ft_sharedptr<int> source_pointer(new int(31));
    ft_sharedptr<int> copied_pointer;
    ft_bool source_lock_acquired;
    ft_bool copy_lock_acquired;

    source_lock_acquired = FT_FALSE;
    copy_lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copied_pointer.initialize(source_pointer));
    FT_ASSERT_EQ(0, source_pointer.enable_thread_safety());
    FT_ASSERT_EQ(0, copied_pointer.enable_thread_safety());
    FT_ASSERT(source_pointer.is_thread_safe());
    FT_ASSERT(copied_pointer.is_thread_safe());
    FT_ASSERT_EQ(2, copied_pointer.use_count());
    FT_ASSERT_EQ(2, source_pointer.use_count());
    FT_ASSERT_EQ(0, source_pointer.lock(&source_lock_acquired));
    FT_ASSERT(source_lock_acquired);
    FT_ASSERT_EQ(0, copied_pointer.lock(&copy_lock_acquired));
    FT_ASSERT(copy_lock_acquired);
    FT_ASSERT_EQ(31, *source_pointer);
    FT_ASSERT_EQ(31, *copied_pointer);
    copied_pointer.unlock(copy_lock_acquired);
    source_pointer.unlock(source_lock_acquired);
    return (1);
}

FT_TEST(test_ft_sharedptr_copy_assignment_recreates_mutex)
{
    ft_sharedptr<int> source_pointer(new int(7));
    ft_sharedptr<int> destination_pointer(new int(3));
    ft_bool source_lock_acquired;
    ft_bool destination_lock_acquired;

    source_lock_acquired = FT_FALSE;
    destination_lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(0, source_pointer.enable_thread_safety());
    FT_ASSERT_EQ(0, destination_pointer.enable_thread_safety());
    FT_ASSERT(source_pointer.is_thread_safe());
    FT_ASSERT(destination_pointer.is_thread_safe());
    destination_pointer = source_pointer;
    FT_ASSERT(destination_pointer.is_thread_safe());
    FT_ASSERT(source_pointer.is_thread_safe());
    FT_ASSERT_EQ(2, destination_pointer.use_count());
    FT_ASSERT_EQ(2, source_pointer.use_count());
    FT_ASSERT_EQ(0, source_pointer.lock(&source_lock_acquired));
    FT_ASSERT(source_lock_acquired);
    FT_ASSERT_EQ(0, destination_pointer.lock(&destination_lock_acquired));
    FT_ASSERT(destination_lock_acquired);
    FT_ASSERT_EQ(7, *destination_pointer);
    FT_ASSERT_EQ(7, *source_pointer);
    destination_pointer.unlock(destination_lock_acquired);
    source_pointer.unlock(source_lock_acquired);
    return (1);
}

FT_TEST(test_ft_sharedptr_move_constructor_rebuilds_mutex)
{
    ft_sharedptr<int> source_pointer(new int(88));
    ft_bool moved_lock_acquired;

    moved_lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(0, source_pointer.enable_thread_safety());
    FT_ASSERT(source_pointer.is_thread_safe());
    ft_sharedptr<int> moved_pointer;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_pointer.move(source_pointer));
    FT_ASSERT(moved_pointer.is_thread_safe());
    FT_ASSERT_EQ(0, moved_pointer.lock(&moved_lock_acquired));
    FT_ASSERT(moved_lock_acquired);
    FT_ASSERT_EQ(88, *moved_pointer);
    moved_pointer.unlock(moved_lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_pointer.initialize());
    FT_ASSERT_EQ(false, source_pointer.is_thread_safe());
    return (1);
}

FT_TEST(test_ft_sharedptr_move_assignment_rebuilds_mutex)
{
    ft_sharedptr<int> destination_pointer(new int(12));
    ft_sharedptr<int> source_pointer(new int(23));
    ft_bool destination_lock_acquired;

    destination_lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(0, destination_pointer.enable_thread_safety());
    FT_ASSERT_EQ(0, source_pointer.enable_thread_safety());
    FT_ASSERT(destination_pointer.is_thread_safe());
    FT_ASSERT(source_pointer.is_thread_safe());
    destination_pointer = ft_move(source_pointer);
    FT_ASSERT(destination_pointer.is_thread_safe());
    FT_ASSERT_EQ(0, destination_pointer.lock(&destination_lock_acquired));
    FT_ASSERT(destination_lock_acquired);
    FT_ASSERT_EQ(23, *destination_pointer);
    destination_pointer.unlock(destination_lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_pointer.initialize());
    FT_ASSERT_EQ(false, source_pointer.is_thread_safe());
    return (1);
}
