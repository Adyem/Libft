#include "../test_internal.hpp"
#include "../../Modules/Template/unordered_map.hpp"
#include "../../Modules/Template/move.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_unordered_map_copy_constructor_recreates_mutex)
{
    ft_unordered_map<int, int> source_map;
    ft_unordered_map<int, int> cloned_map;
    ft_bool source_lock_acquired;
    ft_bool cloned_lock_acquired;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_map.initialize());
    source_map.insert(1, 10);
    source_map.insert(2, 20);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_map.enable_thread_safety());
    FT_ASSERT(source_map.is_thread_safe());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, cloned_map.initialize());
    cloned_map.insert(1, 10);
    cloned_map.insert(2, 20);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cloned_map.enable_thread_safety());
    FT_ASSERT(cloned_map.is_thread_safe());

    source_lock_acquired = FT_FALSE;
    cloned_lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_map.lock(&source_lock_acquired));
    FT_ASSERT(source_lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cloned_map.lock(&cloned_lock_acquired));
    FT_ASSERT(cloned_lock_acquired);
    cloned_map.unlock(cloned_lock_acquired);
    source_map.unlock(source_lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_map.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cloned_map.destroy());
    return (1);
}

FT_TEST(test_ft_unordered_map_copy_assignment_rebuilds_mutex)
{
    ft_unordered_map<int, int> destination_map;
    ft_unordered_map<int, int> source_map;
    ft_bool destination_lock_acquired;
    ft_bool source_lock_acquired;

    destination_lock_acquired = FT_FALSE;
    source_lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_map.initialize());
    destination_map.insert(5, 50);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_map.enable_thread_safety());
    FT_ASSERT(destination_map.is_thread_safe());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_map.initialize());
    source_map.insert(3, 30);
    source_map.insert(4, 40);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_map.enable_thread_safety());
    FT_ASSERT(source_map.is_thread_safe());

    destination_map = source_map;

    FT_ASSERT(destination_map.is_thread_safe());
    FT_ASSERT(source_map.is_thread_safe());
    FT_ASSERT_EQ(2UL, destination_map.size());
    FT_ASSERT_EQ(30, (*destination_map.find(3)).second);
    FT_ASSERT_EQ(40, (*destination_map.find(4)).second);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_map.lock(&destination_lock_acquired));
    FT_ASSERT(destination_lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_map.lock(&source_lock_acquired));
    FT_ASSERT(source_lock_acquired);
    source_map.unlock(source_lock_acquired);
    destination_map.unlock(destination_lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_map.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_map.destroy());
    return (1);
}

FT_TEST(test_ft_unordered_map_move_constructor_resets_source_mutex)
{
    ft_unordered_map<int, int> source_map;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_map.initialize());
    source_map.insert(6, 60);
    source_map.insert(8, 80);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_map.enable_thread_safety());
    FT_ASSERT(source_map.is_thread_safe());

    ft_unordered_map<int, int> moved_map;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_map.initialize());
    moved_map = ft_move(source_map);

    FT_ASSERT(moved_map.is_thread_safe());
    FT_ASSERT_EQ(2UL, moved_map.size());
    FT_ASSERT_EQ(60, (*moved_map.find(6)).second);
    FT_ASSERT_EQ(80, (*moved_map.find(8)).second);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_map.initialize());
    FT_ASSERT(source_map.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_map.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_map.destroy());
    return (1);
}

FT_TEST(test_ft_unordered_map_move_assignment_reinitializes_mutex)
{
    ft_unordered_map<int, int> destination_map;
    ft_unordered_map<int, int> source_map;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_map.initialize());
    destination_map.insert(9, 90);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_map.enable_thread_safety());
    FT_ASSERT(destination_map.is_thread_safe());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_map.initialize());
    source_map.insert(13, 130);
    source_map.insert(17, 170);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_map.enable_thread_safety());
    FT_ASSERT(source_map.is_thread_safe());

    destination_map = ft_move(source_map);

    FT_ASSERT(destination_map.is_thread_safe());
    FT_ASSERT_EQ(2UL, destination_map.size());
    FT_ASSERT_EQ(130, (*destination_map.find(13)).second);
    FT_ASSERT_EQ(170, (*destination_map.find(17)).second);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_map.initialize());
    FT_ASSERT(source_map.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_map.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_map.destroy());
    return (1);
}
