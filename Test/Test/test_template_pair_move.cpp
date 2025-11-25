#include "../../Template/pair.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_pair_copy_constructor_fresh_mutex,
        "Pair copy constructor initializes its own mutex while copying values")
{
    Pair<int, int> original_pair(3, 5);
    Pair<int, int> copied_pair(original_pair);
    bool original_lock_acquired;
    bool copy_lock_acquired;

    original_lock_acquired = false;
    copy_lock_acquired = false;
    FT_ASSERT_EQ(0, original_pair.enable_thread_safety());
    FT_ASSERT(original_pair.is_thread_safe());

    FT_ASSERT(copied_pair.is_thread_safe());
    FT_ASSERT(original_pair.is_thread_safe());
    original_lock_acquired = false;
    FT_ASSERT_EQ(0, original_pair.lock(&original_lock_acquired));
    copy_lock_acquired = false;
    FT_ASSERT_EQ(0, copied_pair.lock(&copy_lock_acquired));
    copied_pair.unlock(copy_lock_acquired);
    original_pair.unlock(original_lock_acquired);

    copied_pair.disable_thread_safety();
    FT_ASSERT_EQ(true, original_pair.is_thread_safe());
    FT_ASSERT_EQ(false, copied_pair.is_thread_safe());
    FT_ASSERT_EQ(3, copied_pair.key);
    FT_ASSERT_EQ(5, copied_pair.value);
    return (1);
}

FT_TEST(test_pair_move_constructor_rebuilds_mutex,
        "Pair move constructor rebuilds thread safety and transfers values")
{
    Pair<int, int> source_pair(7, 9);
    Pair<int, int> moved_pair;

    FT_ASSERT_EQ(0, source_pair.enable_thread_safety());
    FT_ASSERT(source_pair.is_thread_safe());

    moved_pair = Pair<int, int>(ft_move(source_pair));

    FT_ASSERT(moved_pair.is_thread_safe());
    FT_ASSERT_EQ(false, source_pair.is_thread_safe());
    FT_ASSERT_EQ(7, moved_pair.key);
    FT_ASSERT_EQ(9, moved_pair.value);
    FT_ASSERT_EQ(0, source_pair.enable_thread_safety());
    FT_ASSERT(source_pair.is_thread_safe());
    source_pair.set_key(1);
    source_pair.set_value(2);
    FT_ASSERT_EQ(1, source_pair.key);
    FT_ASSERT_EQ(2, source_pair.value);
    return (1);
}

FT_TEST(test_pair_move_assignment_rebuilds_mutex,
        "Pair move assignment reinitializes thread safety and replaces contents")
{
    Pair<int, int> destination_pair(2, 4);
    Pair<int, int> source_pair(12, 14);

    FT_ASSERT_EQ(0, destination_pair.enable_thread_safety());
    FT_ASSERT_EQ(0, source_pair.enable_thread_safety());
    FT_ASSERT(destination_pair.is_thread_safe());
    FT_ASSERT(source_pair.is_thread_safe());

    destination_pair = ft_move(source_pair);

    FT_ASSERT(destination_pair.is_thread_safe());
    FT_ASSERT_EQ(false, source_pair.is_thread_safe());
    FT_ASSERT_EQ(12, destination_pair.key);
    FT_ASSERT_EQ(14, destination_pair.value);
    FT_ASSERT_EQ(0, source_pair.enable_thread_safety());
    FT_ASSERT(source_pair.is_thread_safe());
    source_pair.set_key(5);
    source_pair.set_value(6);
    FT_ASSERT_EQ(5, source_pair.key);
    FT_ASSERT_EQ(6, source_pair.value);
    return (1);
}
