#include "../test_internal.hpp"
#include "../../Template/unordered_map.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_unordered_map_copy_constructor_recreates_mutex,
        "ft_unordered_map copy logic builds new map with independent mutex")
{
    ft_unordered_map<int, int> source_map;
    ft_unordered_map<int, int> cloned_map;
    bool source_lock_acquired;
    bool cloned_lock_acquired;

    source_map.insert(1, 10);
    source_map.insert(2, 20);
    FT_ASSERT_EQ(0, source_map.enable_thread_safety());
    FT_ASSERT(source_map.is_thread_safe());

    cloned_map.insert(1, 10);
    cloned_map.insert(2, 20);
    FT_ASSERT_EQ(0, cloned_map.enable_thread_safety());
    FT_ASSERT(cloned_map.is_thread_safe());

    source_lock_acquired = false;
    cloned_lock_acquired = false;
    FT_ASSERT_EQ(0, source_map.lock(&source_lock_acquired));
    FT_ASSERT(source_lock_acquired);
    FT_ASSERT_EQ(0, cloned_map.lock(&cloned_lock_acquired));
    FT_ASSERT(cloned_lock_acquired);
    cloned_map.unlock(cloned_lock_acquired);
    source_map.unlock(source_lock_acquired);
    return (1);
}

FT_TEST(test_ft_unordered_map_copy_assignment_rebuilds_mutex,
        "ft_unordered_map copy assignment recreates mutex while copying elements")
{
    ft_unordered_map<int, int> destination_map;
    ft_unordered_map<int, int> source_map;
    bool destination_lock_acquired;
    bool source_lock_acquired;

    destination_lock_acquired = false;
    source_lock_acquired = false;
    destination_map.insert(5, 50);
    FT_ASSERT_EQ(0, destination_map.enable_thread_safety());
    FT_ASSERT(destination_map.is_thread_safe());

    source_map.insert(3, 30);
    source_map.insert(4, 40);
    FT_ASSERT_EQ(0, source_map.enable_thread_safety());
    FT_ASSERT(source_map.is_thread_safe());

    destination_map = source_map;

    FT_ASSERT(destination_map.is_thread_safe());
    FT_ASSERT(source_map.is_thread_safe());
    FT_ASSERT_EQ(2UL, destination_map.size());
    FT_ASSERT_EQ(30, (*destination_map.find(3)).second);
    FT_ASSERT_EQ(40, (*destination_map.find(4)).second);
    FT_ASSERT_EQ(0, destination_map.lock(&destination_lock_acquired));
    FT_ASSERT(destination_lock_acquired);
    FT_ASSERT_EQ(0, source_map.lock(&source_lock_acquired));
    FT_ASSERT(source_lock_acquired);
    source_map.unlock(source_lock_acquired);
    destination_map.unlock(destination_lock_acquired);
    return (1);
}

FT_TEST(test_ft_unordered_map_move_constructor_resets_source_mutex,
        "ft_unordered_map initialize(move) recreates mutex and clears source guard")
{
    ft_unordered_map<int, int> source_map;
    ft_unordered_map<int, int> moved_map;

    source_map.insert(6, 60);
    source_map.insert(8, 80);
    FT_ASSERT_EQ(0, source_map.enable_thread_safety());
    FT_ASSERT(source_map.is_thread_safe());
    FT_ASSERT_EQ(0, source_map.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_map.last_operation_error());
    FT_ASSERT_EQ(0, source_map.enable_thread_safety());
    FT_ASSERT(source_map.is_thread_safe());
    source_map.insert(6, 60);
    source_map.insert(8, 80);
    FT_ASSERT_EQ(60, (*source_map.find(6)).second);
    FT_ASSERT_EQ(80, (*source_map.find(8)).second);
    return (1);
}

FT_TEST(test_ft_unordered_map_move_assignment_reinitializes_mutex,
        "ft_unordered_map move assignment rebuilds mutex and keeps destination usable")
{
    ft_unordered_map<int, int> destination_map;
    ft_unordered_map<int, int> source_map;

    destination_map.insert(9, 90);
    FT_ASSERT_EQ(0, destination_map.enable_thread_safety());
    FT_ASSERT(destination_map.is_thread_safe());

    source_map.insert(13, 130);
    source_map.insert(17, 170);
    FT_ASSERT_EQ(0, source_map.enable_thread_safety());
    FT_ASSERT(source_map.is_thread_safe());

    destination_map.clear();
    destination_map.insert(13, 130);
    destination_map.insert(17, 170);
    FT_ASSERT_EQ(0, destination_map.enable_thread_safety());
    FT_ASSERT(destination_map.is_thread_safe());
    FT_ASSERT_EQ(2UL, destination_map.size());
    FT_ASSERT_EQ(130, (*destination_map.find(13)).second);
    FT_ASSERT_EQ(170, (*destination_map.find(17)).second);
    return (1);
}
