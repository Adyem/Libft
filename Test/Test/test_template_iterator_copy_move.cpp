#include "../test_internal.hpp"
#include "../../Modules/Template/iterator.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <utility>

#ifndef LIBFT_TEST_BUILD
#endif

static int iterator_expect_sigabrt(void (*operation)(void))
{
    return (test_expect_sigabrt_signal(operation));
}

static ft_bool g_iterator_get_error_returned = FT_FALSE;
static int32_t g_iterator_get_error_result = FT_ERR_SUCCESS;
static ft_bool g_iterator_get_error_str_returned = FT_FALSE;
static const char *g_iterator_get_error_str_result = ft_nullptr;

static void iterator_get_error_uninitialised_operation(void)
{
    Iterator<int> iterator_value;

    g_iterator_get_error_result = iterator_value.get_error();
    g_iterator_get_error_returned = FT_TRUE;
    return ;
}

static void iterator_get_error_str_uninitialised_operation(void)
{
    Iterator<int> iterator_value;

    g_iterator_get_error_str_result = iterator_value.get_error_str();
    g_iterator_get_error_str_returned = FT_TRUE;
    return ;
}

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

FT_TEST(test_iterator_error_queries_follow_lifecycle_contract)
{
    int values[1] = {4};
    Iterator<int> iterator_value(values);

    g_iterator_get_error_returned = FT_FALSE;
    g_iterator_get_error_result = FT_ERR_SUCCESS;
    g_iterator_get_error_str_returned = FT_FALSE;
    g_iterator_get_error_str_result = ft_nullptr;
    FT_ASSERT_EQ(1, iterator_expect_sigabrt(
        iterator_get_error_uninitialised_operation));
    FT_ASSERT_EQ(FT_FALSE, g_iterator_get_error_returned);
    FT_ASSERT_EQ(1, iterator_expect_sigabrt(
        iterator_get_error_str_uninitialised_operation));
    FT_ASSERT_EQ(FT_FALSE, g_iterator_get_error_str_returned);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, iterator_value.get_error());
    FT_ASSERT_EQ(0, iterator_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, iterator_value.get_error());
    FT_ASSERT_EQ(0, ft_strcmp(iterator_value.get_error_str(),
        ft_strerror(FT_ERR_SUCCESS)));
    return (1);
}
