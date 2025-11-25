#include "../../Template/tuple.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_ft_tuple_move_constructor_rebuilds_mutex,
        "ft_tuple move constructor recreates mutex and preserves element values")
{
    ft_tuple<int, int> source_tuple(5, 7);
    bool moved_lock_acquired;
    bool source_lock_acquired;

    moved_lock_acquired = false;
    source_lock_acquired = false;
    FT_ASSERT_EQ(0, source_tuple.enable_thread_safety());
    FT_ASSERT(source_tuple.is_thread_safe());
    ft_tuple<int, int> moved_tuple(ft_move(source_tuple));
    FT_ASSERT(moved_tuple.is_thread_safe());
    FT_ASSERT_EQ(false, source_tuple.is_thread_safe());
    FT_ASSERT_EQ(0, moved_tuple.lock(&moved_lock_acquired));
    FT_ASSERT(moved_lock_acquired);
    FT_ASSERT_EQ(5, moved_tuple.get<0>());
    FT_ASSERT_EQ(7, moved_tuple.get<1>());
    moved_tuple.unlock(moved_lock_acquired);
    FT_ASSERT_EQ(0, source_tuple.enable_thread_safety());
    FT_ASSERT(source_tuple.is_thread_safe());
    FT_ASSERT_EQ(0, source_tuple.lock(&source_lock_acquired));
    FT_ASSERT(source_lock_acquired);
    source_tuple.unlock(source_lock_acquired);
    return (1);
}

FT_TEST(test_ft_tuple_move_assignment_rebuilds_mutex,
        "ft_tuple move assignment reinstates mutex and keeps moved-from reusable")
{
    ft_tuple<int, int> source_tuple(11, 13);
    ft_tuple<int, int> destination_tuple(1, 2);
    bool destination_lock_acquired;
    bool source_lock_acquired;

    destination_lock_acquired = false;
    source_lock_acquired = false;
    FT_ASSERT_EQ(0, source_tuple.enable_thread_safety());
    FT_ASSERT_EQ(0, destination_tuple.enable_thread_safety());
    destination_tuple = ft_move(source_tuple);
    FT_ASSERT(destination_tuple.is_thread_safe());
    FT_ASSERT_EQ(false, source_tuple.is_thread_safe());
    FT_ASSERT_EQ(0, destination_tuple.lock(&destination_lock_acquired));
    FT_ASSERT(destination_lock_acquired);
    FT_ASSERT_EQ(11, destination_tuple.get<0>());
    FT_ASSERT_EQ(13, destination_tuple.get<1>());
    destination_tuple.unlock(destination_lock_acquired);
    FT_ASSERT_EQ(0, source_tuple.enable_thread_safety());
    FT_ASSERT(source_tuple.is_thread_safe());
    FT_ASSERT_EQ(0, source_tuple.lock(&source_lock_acquired));
    FT_ASSERT(source_lock_acquired);
    source_tuple.unlock(source_lock_acquired);
    return (1);
}
