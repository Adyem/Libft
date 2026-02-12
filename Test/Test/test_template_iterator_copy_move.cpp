#include "../test_internal.hpp"
#include "../../Template/iterator.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_iterator_copy_constructor_rebuilds_mutex,
        "Iterator copy constructor recreates mutex and keeps pointer")
{
    int     values[2];
    Iterator<int> source_iterator(values);

    values[0] = 5;
    values[1] = 8;
    FT_ASSERT_EQ(0, source_iterator.enable_thread_safety());
    FT_ASSERT(source_iterator.is_thread_safe_enabled());

    Iterator<int> copied_iterator(source_iterator);

    FT_ASSERT(copied_iterator.is_thread_safe_enabled());
    FT_ASSERT(source_iterator.is_thread_safe_enabled());
    FT_ASSERT_EQ(values[0], *copied_iterator);
    copied_iterator = copied_iterator.operator++();
    FT_ASSERT_EQ(values[1], *copied_iterator);
    return (1);
}

FT_TEST(test_iterator_copy_assignment_follows_thread_safety,
        "Iterator copy assignment initializes mutex and copies address")
{
    int             values_one[1];
    int             values_two[1];
    Iterator<int>   destination_iterator(values_one);
    Iterator<int>   source_iterator(values_two);

    values_one[0] = 1;
    values_two[0] = 3;
    FT_ASSERT_EQ(0, destination_iterator.enable_thread_safety());
    FT_ASSERT(destination_iterator.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, source_iterator.enable_thread_safety());
    FT_ASSERT(source_iterator.is_thread_safe_enabled());

    destination_iterator = source_iterator;

    FT_ASSERT(destination_iterator.is_thread_safe_enabled());
    FT_ASSERT(source_iterator.is_thread_safe_enabled());
    FT_ASSERT_EQ(values_two[0], *destination_iterator);
    return (1);
}

FT_TEST(test_iterator_move_constructor_resets_source_mutex,
        "Iterator move constructor rebuilds mutex and transfers pointer")
{
    int             values[2];
    Iterator<int>   source_iterator(values);

    values[0] = 9;
    values[1] = 10;
    FT_ASSERT_EQ(0, source_iterator.enable_thread_safety());
    FT_ASSERT(source_iterator.is_thread_safe_enabled());

    Iterator<int> moved_iterator(ft_move(source_iterator));

    FT_ASSERT(moved_iterator.is_thread_safe_enabled());
    FT_ASSERT_EQ(false, source_iterator.is_thread_safe_enabled());
    FT_ASSERT_EQ(values[0], *moved_iterator);
    moved_iterator = moved_iterator.operator++();
    FT_ASSERT_EQ(values[1], *moved_iterator);
    return (1);
}

FT_TEST(test_iterator_move_assignment_recreates_mutex,
        "Iterator move assignment recreates mutex and updates pointer")
{
    int             values_one[1];
    int             values_two[1];
    Iterator<int>   destination_iterator(values_one);
    Iterator<int>   source_iterator(values_two);

    values_one[0] = 7;
    values_two[0] = 12;
    FT_ASSERT_EQ(0, destination_iterator.enable_thread_safety());
    FT_ASSERT(destination_iterator.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, source_iterator.enable_thread_safety());
    FT_ASSERT(source_iterator.is_thread_safe_enabled());

    destination_iterator = ft_move(source_iterator);

    FT_ASSERT(destination_iterator.is_thread_safe_enabled());
    FT_ASSERT_EQ(false, source_iterator.is_thread_safe_enabled());
    FT_ASSERT_EQ(values_two[0], *destination_iterator);
    FT_ASSERT_EQ(0, source_iterator.enable_thread_safety());
    FT_ASSERT(source_iterator.is_thread_safe_enabled());
    return (1);
}
