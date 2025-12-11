#include "../../Template/map.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_map_thread_safety_resets_errno,
        "ft_map toggles optional mutex guard and resets errno to success during locks")
{
    ft_map<int, int> map_instance;
    bool lock_acquired;

    FT_ASSERT_EQ(false, map_instance.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, map_instance.enable_thread_safety());
    FT_ASSERT_EQ(true, map_instance.is_thread_safe_enabled());
    map_instance.insert(1, 2);
    ft_errno = FT_ERR_INVALID_OPERATION;
    lock_acquired = false;
    FT_ASSERT_EQ(0, map_instance.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    map_instance.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    map_instance.disable_thread_safety();
    FT_ASSERT_EQ(false, map_instance.is_thread_safe_enabled());
    return (1);
}

FT_TEST(test_map_move_transfers_thread_safety,
        "ft_map move operations transfer thread-safety guards with stored values")
{
    ft_map<int, int> original;

    original.insert(3, 9);
    FT_ASSERT_EQ(0, original.enable_thread_safety());
    FT_ASSERT_EQ(true, original.is_thread_safe_enabled());

    ft_map<int, int> moved(ft_move(original));
    FT_ASSERT_EQ(true, moved.is_thread_safe_enabled());
    FT_ASSERT_EQ(false, original.is_thread_safe_enabled());
    Pair<int, int> *found_entry;

    found_entry = moved.find(3);
    FT_ASSERT(found_entry != ft_nullptr);
    FT_ASSERT_EQ(9, found_entry->value);

    ft_map<int, int> assigned;
    assigned.insert(5, 25);
    FT_ASSERT_EQ(0, assigned.enable_thread_safety());
    FT_ASSERT_EQ(true, assigned.is_thread_safe_enabled());
    assigned = ft_move(moved);
    FT_ASSERT_EQ(true, assigned.is_thread_safe_enabled());
    FT_ASSERT_EQ(false, moved.is_thread_safe_enabled());
    found_entry = assigned.find(3);
    FT_ASSERT(found_entry != ft_nullptr);
    FT_ASSERT_EQ(9, found_entry->value);

    return (1);
}

FT_TEST(test_map_copy_retains_thread_safety,
        "ft_map copy constructor duplicates data and mutex guard when available")
{
    ft_map<int, int> source;

    source.insert(7, 49);
    FT_ASSERT_EQ(0, source.enable_thread_safety());
    FT_ASSERT_EQ(true, source.is_thread_safe_enabled());

    ft_map<int, int> copy(source);
    FT_ASSERT_EQ(true, source.is_thread_safe_enabled());
    FT_ASSERT_EQ(true, copy.is_thread_safe_enabled());
    Pair<int, int> *copied_entry;

    copied_entry = copy.find(7);
    FT_ASSERT(copied_entry != ft_nullptr);
    FT_ASSERT_EQ(49, copied_entry->value);

    return (1);
}
