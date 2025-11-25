#include "../../Template/unordered_map.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_ft_unordered_map_copy_constructor_recreates_mutex,
        "ft_unordered_map copy constructor rebuilds mutex for independent locking")
{
    ft_unordered_map<int, int> source_map;
    bool source_lock_acquired;
    bool copy_lock_acquired;

    source_lock_acquired = false;
    copy_lock_acquired = false;
    source_map.insert(1, 10);
    source_map.insert(2, 20);
    FT_ASSERT_EQ(0, source_map.enable_thread_safety());
    FT_ASSERT(source_map.is_thread_safe());

    ft_unordered_map<int, int> copied_map(source_map);

    FT_ASSERT(copied_map.is_thread_safe());
    FT_ASSERT(source_map.is_thread_safe());
    FT_ASSERT_EQ(2UL, copied_map.size());
    FT_ASSERT_EQ(10, (*copied_map.find(1)).second);
    FT_ASSERT_EQ(20, (*copied_map.find(2)).second);
    FT_ASSERT_EQ(0, source_map.lock(&source_lock_acquired));
    FT_ASSERT(source_lock_acquired);
    FT_ASSERT_EQ(0, copied_map.lock(&copy_lock_acquired));
    FT_ASSERT(copy_lock_acquired);
    copied_map.unlock(copy_lock_acquired);
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
        "ft_unordered_map move constructor recreates mutex and clears source guard")
{
    ft_unordered_map<int, int> source_map;

    source_map.insert(6, 60);
    source_map.insert(8, 80);
    FT_ASSERT_EQ(0, source_map.enable_thread_safety());
    FT_ASSERT(source_map.is_thread_safe());

    ft_unordered_map<int, int> moved_map(ft_move(source_map));

    FT_ASSERT(moved_map.is_thread_safe());
    FT_ASSERT_EQ(false, source_map.is_thread_safe());
    FT_ASSERT_EQ(2UL, moved_map.size());
    FT_ASSERT_EQ(60, (*moved_map.find(6)).second);
    FT_ASSERT_EQ(80, (*moved_map.find(8)).second);
    FT_ASSERT_EQ(0, source_map.enable_thread_safety());
    FT_ASSERT(source_map.is_thread_safe());
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

    destination_map = ft_move(source_map);

    FT_ASSERT(destination_map.is_thread_safe());
    FT_ASSERT_EQ(false, source_map.is_thread_safe());
    FT_ASSERT_EQ(2UL, destination_map.size());
    FT_ASSERT_EQ(130, (*destination_map.find(13)).second);
    FT_ASSERT_EQ(170, (*destination_map.find(17)).second);
    FT_ASSERT_EQ(0, source_map.enable_thread_safety());
    FT_ASSERT(source_map.is_thread_safe());
    return (1);
}
