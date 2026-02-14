#include "../test_internal.hpp"
#include "../../Template/map.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_map_thread_safety_toggles_mutex,
        "ft_map toggles optional mutex guard and locks successfully")
{
    ft_map<int, int> map_instance;
    bool lock_acquired;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.initialize());
    FT_ASSERT_EQ(true, map_instance.is_thread_safe());
    map_instance.insert(1, 2);
    lock_acquired = false;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.unlock(lock_acquired));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.disable_thread_safety());
    FT_ASSERT_EQ(false, map_instance.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.enable_thread_safety());
    FT_ASSERT_EQ(true, map_instance.is_thread_safe());
    return (1);
}

FT_TEST(test_map_move_transfers_thread_safety,
        "ft_map move operations transfer thread-safety guards with stored values")
{
    ft_map<int, int> original;
    ft_map<int, int> moved;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.initialize());
    original.insert(3, 9);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.move_from(original));
    Pair<int, int> *found_entry;

    found_entry = moved.find(3);
    FT_ASSERT(found_entry != ft_nullptr);
    FT_ASSERT_EQ(9, found_entry->value);

    ft_map<int, int> assigned;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, assigned.initialize());
    assigned.insert(5, 25);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, assigned.move_from(moved));
    found_entry = assigned.find(3);
    FT_ASSERT(found_entry != ft_nullptr);
    FT_ASSERT_EQ(9, found_entry->value);

    return (1);
}

FT_TEST(test_map_copy_retains_thread_safety,
        "ft_map copy constructor duplicates data and mutex guard when available")
{
    ft_map<int, int> source;
    ft_map<int, int> copy;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    source.insert(7, 49);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.copy_from(source));
    Pair<int, int> *copied_entry;

    copied_entry = copy.find(7);
    FT_ASSERT(copied_entry != ft_nullptr);
    FT_ASSERT_EQ(49, copied_entry->value);

    return (1);
}
