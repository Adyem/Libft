#include "../test_internal.hpp"
#include "../../Template/iterator.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include <utility>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_iterator_manual_copy_recreates_mutex)
{
    int values[2] = {5, 8};
    Iterator<int> source_iterator(values);
    Iterator<int> copy_iterator;

    FT_ASSERT_EQ(0, source_iterator.enable_thread_safety());
    FT_ASSERT(source_iterator.is_thread_safe());
    FT_ASSERT_EQ(0, copy_iterator.initialize(values));
    FT_ASSERT_EQ(0, copy_iterator.enable_thread_safety());
    FT_ASSERT(copy_iterator.is_thread_safe());
    FT_ASSERT_EQ(values[0], *copy_iterator);
    (void)copy_iterator.operator++();
    FT_ASSERT_EQ(values[1], *copy_iterator);
    return (1);
}

FT_TEST(test_iterator_manual_assignment_rebuilds_mutex)
{
    int values_one[1] = {1};
    int values_two[1] = {3};
    Iterator<int> destination_iterator(values_one);
    Iterator<int> source_iterator(values_two);

    FT_ASSERT_EQ(0, destination_iterator.enable_thread_safety());
    FT_ASSERT(destination_iterator.is_thread_safe());
    FT_ASSERT_EQ(0, destination_iterator.destroy());
    FT_ASSERT_EQ(0, destination_iterator.initialize(values_two));
    FT_ASSERT_EQ(0, destination_iterator.enable_thread_safety());
    FT_ASSERT(destination_iterator.is_thread_safe());
    FT_ASSERT_EQ(values_two[0], *destination_iterator);
    return (1);
}

FT_TEST(test_iterator_manual_move_rebuilds_mutex)
{
    int values[2] = {9, 10};
    Iterator<int> source_iterator(values);
    Iterator<int> moved_iterator;

    FT_ASSERT_EQ(0, source_iterator.enable_thread_safety());
    FT_ASSERT(source_iterator.is_thread_safe());
    FT_ASSERT_EQ(0, moved_iterator.move(source_iterator));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_iterator.get_error());
    FT_ASSERT(moved_iterator.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_iterator.get_error());
    FT_ASSERT_EQ(values[0], *moved_iterator);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_iterator.get_error());
    (void)moved_iterator.operator++();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_iterator.get_error());
    FT_ASSERT_EQ(values[1], *moved_iterator);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_iterator.get_error());
    FT_ASSERT_EQ(0, source_iterator.initialize(values));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_iterator.get_error());
    FT_ASSERT_EQ(values[0], *source_iterator);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_iterator.get_error());
    return (1);
}

FT_TEST(test_iterator_move_into_initialized_destination_preserves_source_thread_safety)
{
    int values_one[1] = {7};
    int values_two[1] = {12};
    Iterator<int> destination_iterator(values_one);
    Iterator<int> source_iterator(values_two);

    FT_ASSERT_EQ(0, destination_iterator.enable_thread_safety());
    FT_ASSERT(destination_iterator.is_thread_safe());
    FT_ASSERT_EQ(0, destination_iterator.move(source_iterator));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_iterator.get_error());
    FT_ASSERT_EQ(FT_FALSE, destination_iterator.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_iterator.get_error());
    FT_ASSERT_EQ(values_two[0], *destination_iterator);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_iterator.get_error());
    FT_ASSERT_EQ(0, source_iterator.initialize(values_one));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_iterator.get_error());
    FT_ASSERT_EQ(values_one[0], *source_iterator);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_iterator.get_error());
    return (1);
}
