#include "../../Template/set.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_ft_set_move_constructor_rebuilds_mutex,
        "ft_set move constructor recreates thread-safety while copying data")
{
    ft_set<int> source_set;

    source_set.insert(5);
    source_set.insert(9);
    FT_ASSERT_EQ(0, source_set.enable_thread_safety());
    FT_ASSERT(source_set.is_thread_safe());

    ft_set<int> moved_set(ft_move(source_set));

    FT_ASSERT(moved_set.is_thread_safe());
    FT_ASSERT_EQ(false, source_set.is_thread_safe());
    FT_ASSERT_EQ(2UL, moved_set.size());
    FT_ASSERT_NE(ft_nullptr, moved_set.find(5));
    FT_ASSERT_NE(ft_nullptr, moved_set.find(9));
    return (1);
}

FT_TEST(test_ft_set_move_assignment_rebuilds_mutex,
        "ft_set move assignment reinitializes mutex and replaces elements")
{
    ft_set<int> destination_set;
    ft_set<int> source_set;

    destination_set.insert(3);
    destination_set.insert(4);
    FT_ASSERT_EQ(0, destination_set.enable_thread_safety());
    FT_ASSERT(destination_set.is_thread_safe());

    source_set.insert(11);
    source_set.insert(18);
    FT_ASSERT_EQ(0, source_set.enable_thread_safety());
    FT_ASSERT(source_set.is_thread_safe());

    destination_set = ft_move(source_set);

    FT_ASSERT(destination_set.is_thread_safe());
    FT_ASSERT_EQ(false, source_set.is_thread_safe());
    FT_ASSERT_EQ(2UL, destination_set.size());
    FT_ASSERT_EQ(ft_nullptr, destination_set.find(3));
    FT_ASSERT_EQ(ft_nullptr, destination_set.find(4));
    FT_ASSERT_NE(ft_nullptr, destination_set.find(11));
    FT_ASSERT_NE(ft_nullptr, destination_set.find(18));
    FT_ASSERT_EQ(ER_SUCCESS, destination_set.get_error());
    return (1);
}

FT_TEST(test_ft_set_move_preserves_disabled_thread_safety,
        "ft_set move constructor keeps thread-safety disabled when source was unlocked")
{
    ft_set<int> source_set;

    source_set.insert(21);
    source_set.insert(34);
    FT_ASSERT_EQ(false, source_set.is_thread_safe());

    ft_set<int> moved_set(ft_move(source_set));

    FT_ASSERT_EQ(false, moved_set.is_thread_safe());
    FT_ASSERT_EQ(false, source_set.is_thread_safe());
    FT_ASSERT_EQ(2UL, moved_set.size());
    FT_ASSERT_NE(ft_nullptr, moved_set.find(21));
    FT_ASSERT_NE(ft_nullptr, moved_set.find(34));
    return (1);
}

FT_TEST(test_ft_set_move_allows_reinitializing_source_mutex,
        "ft_set moved-from object can enable thread-safety again")
{
    ft_set<int> source_set;
    ft_set<int> moved_set;

    source_set.insert(13);
    source_set.insert(17);
    FT_ASSERT_EQ(0, source_set.enable_thread_safety());
    FT_ASSERT(source_set.is_thread_safe());

    moved_set = ft_move(source_set);

    FT_ASSERT(moved_set.is_thread_safe());
    FT_ASSERT_EQ(false, source_set.is_thread_safe());
    FT_ASSERT_NE(ft_nullptr, moved_set.find(13));
    FT_ASSERT_NE(ft_nullptr, moved_set.find(17));

    FT_ASSERT_EQ(0, source_set.enable_thread_safety());
    FT_ASSERT(source_set.is_thread_safe());
    source_set.insert(1);
    FT_ASSERT_NE(ft_nullptr, source_set.find(1));
    return (1);
}
