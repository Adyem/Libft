#include "../test_internal.hpp"
#include "../../Modules/Template/set.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

using set_type = ft_set<int>;

static int set_expect_sigabrt(void (*operation)(void))
{
    return (test_expect_sigabrt_signal(operation));
}

static ft_bool g_set_get_error_returned = FT_FALSE;
static int32_t g_set_get_error_result = FT_ERR_SUCCESS;
static ft_bool g_set_get_error_str_returned = FT_FALSE;
static const char *g_set_get_error_str_result = ft_nullptr;

static void set_get_error_uninitialised_operation(void)
{
    set_type set_value;

    g_set_get_error_result = set_value.get_error();
    g_set_get_error_returned = FT_TRUE;
    return ;
}

static void set_get_error_str_uninitialised_operation(void)
{
    set_type set_value;

    g_set_get_error_str_result = set_value.get_error_str();
    g_set_get_error_str_returned = FT_TRUE;
    return ;
}

FT_TEST(test_ft_set_move_constructor_rebuilds_mutex)
{
    set_type source_set;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_set.initialize());
    source_set.insert(3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_set.get_error());
    source_set.insert(7);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_set.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_set.enable_thread_safety());
    FT_ASSERT(source_set.is_thread_safe());

    set_type moved_set;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_set.move(source_set));

    FT_ASSERT(moved_set.is_thread_safe());
    FT_ASSERT_EQ(2UL, moved_set.size());
    FT_ASSERT(moved_set.find(3) != ft_nullptr);
    FT_ASSERT(moved_set.find(7) != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_set.initialize());
    FT_ASSERT_EQ(false, source_set.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_set.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_set.destroy());
    return (1);
}

FT_TEST(test_ft_set_move_assignment_rebuilds_mutex)
{
    set_type destination_set;
    set_type source_set;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_set.initialize());
    destination_set.insert(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_set.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_set.enable_thread_safety());
    FT_ASSERT(destination_set.is_thread_safe());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_set.initialize());
    source_set.insert(9);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_set.get_error());
    source_set.insert(11);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_set.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_set.enable_thread_safety());
    FT_ASSERT(source_set.is_thread_safe());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_set.move(source_set));
    FT_ASSERT(destination_set.is_thread_safe());
    FT_ASSERT_EQ(2UL, destination_set.size());
    FT_ASSERT(destination_set.find(9) != ft_nullptr);
    FT_ASSERT(destination_set.find(11) != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_set.initialize());
    FT_ASSERT_EQ(false, source_set.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_set.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_set.destroy());
    return (1);
}

FT_TEST(test_ft_set_move_preserves_disabled_thread_safety)
{
    set_type source_set;
    set_type destination_set;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_set.initialize());
    source_set.insert(5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_set.get_error());
    source_set.insert(9);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_set.get_error());
    FT_ASSERT_EQ(false, source_set.is_thread_safe());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_set.move(source_set));
    FT_ASSERT_EQ(false, destination_set.is_thread_safe());
    FT_ASSERT_EQ(2UL, destination_set.size());
    FT_ASSERT(destination_set.find(5) != ft_nullptr);
    FT_ASSERT(destination_set.find(9) != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_set.initialize());
    FT_ASSERT_EQ(false, source_set.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_set.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_set.destroy());
    return (1);
}

FT_TEST(test_ft_set_move_allows_reinitializing_source_mutex)
{
    set_type source_set;
    set_type destination_set;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_set.initialize());
    source_set.insert(8);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_set.get_error());
    source_set.insert(12);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_set.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_set.enable_thread_safety());
    FT_ASSERT(source_set.is_thread_safe());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_set.move(source_set));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_set.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_set.enable_thread_safety());
    FT_ASSERT(source_set.is_thread_safe());
    source_set.insert(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_set.get_error());
    FT_ASSERT_EQ(1UL, source_set.size());
    FT_ASSERT(source_set.find(1) != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_set.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_set.destroy());
    return (1);
}

FT_TEST(test_ft_set_error_queries_follow_lifecycle_contract)
{
    set_type set_value;

    g_set_get_error_returned = FT_FALSE;
    g_set_get_error_result = FT_ERR_SUCCESS;
    g_set_get_error_str_returned = FT_FALSE;
    g_set_get_error_str_result = ft_nullptr;
    FT_ASSERT_EQ(1, set_expect_sigabrt(
        set_get_error_uninitialised_operation));
    FT_ASSERT_EQ(FT_FALSE, g_set_get_error_returned);
    FT_ASSERT_EQ(1, set_expect_sigabrt(
        set_get_error_str_uninitialised_operation));
    FT_ASSERT_EQ(FT_FALSE, g_set_get_error_str_returned);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, set_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, set_value.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, set_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, set_value.get_error());
    FT_ASSERT_EQ(0, ft_strcmp(set_value.get_error_str(),
        ft_strerror(FT_ERR_SUCCESS)));
    return (1);
}
