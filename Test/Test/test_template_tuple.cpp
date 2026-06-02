#include "../test_internal.hpp"
#include "../../Modules/Template/tuple.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/CPP_class/class_string.hpp"
#include "../../Modules/Template/move.hpp"


#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

using triple_tuple = ft_tuple<int, ft_string, double>;
using duo_tuple = ft_tuple<int, ft_string>;

static int tuple_expect_sigabrt(void (*operation)(void))
{
    return (test_expect_sigabrt_signal(operation));
}

static ft_bool g_tuple_get_error_returned = FT_FALSE;
static int32_t g_tuple_get_error_result = FT_ERR_SUCCESS;
static ft_bool g_tuple_get_error_str_returned = FT_FALSE;
static const char *g_tuple_get_error_str_result = ft_nullptr;

static void tuple_get_error_uninitialised_operation(void)
{
    duo_tuple tuple_value;

    g_tuple_get_error_result = tuple_value.get_error();
    g_tuple_get_error_returned = FT_TRUE;
    return ;
}

static void tuple_get_error_str_uninitialised_operation(void)
{
    duo_tuple tuple_value;

    g_tuple_get_error_str_result = tuple_value.get_error_str();
    g_tuple_get_error_str_returned = FT_TRUE;
    return ;
}

FT_TEST(test_ft_tuple_construction_and_get)
{
    ft_string second_string;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second_string.initialize("value"));
    triple_tuple tuple_instance;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tuple_instance.initialize());
    tuple_instance.get<0>() = 7;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tuple_instance.get<1>().initialize(second_string));
    tuple_instance.get<2>() = 3.5;

    int first_value = tuple_instance.get<0>();
    FT_ASSERT_EQ(7, first_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tuple_instance.get_error());

    ft_string &second_value = tuple_instance.get<1>();
    FT_ASSERT_STR_EQ("value", second_value.c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tuple_instance.get_error());

    double third_value = tuple_instance.get<2>();
    FT_ASSERT_EQ(3.5, third_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tuple_instance.get_error());

    double type_lookup = tuple_instance.get<double>();
    FT_ASSERT_EQ(3.5, type_lookup);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tuple_instance.get_error());
    return (1);
}

FT_TEST(test_ft_tuple_reset_and_error_reporting)
{
    ft_string answer_string;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, answer_string.initialize("answer"));
    duo_tuple tuple_instance;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tuple_instance.initialize());
    tuple_instance.get<0>() = 42;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tuple_instance.get<1>().initialize(answer_string));

    tuple_instance.reset();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tuple_instance.get_error());

    int missing_value = tuple_instance.get<0>();
    FT_ASSERT_EQ(0, missing_value);
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, tuple_instance.get_error());

    ft_string &missing_string = tuple_instance.get<1>();
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, tuple_instance.get_error());
    FT_ASSERT_EQ(FT_FALSE, missing_string.is_initialised());

    ft_string eleven_string;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, eleven_string.initialize("eleven"));
    duo_tuple refreshed_tuple;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, refreshed_tuple.initialize());
    refreshed_tuple.get<0>() = 11;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, refreshed_tuple.get<1>().initialize(eleven_string));
    tuple_instance.reset();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tuple_instance.get_error());
    FT_ASSERT_EQ(11, refreshed_tuple.get<0>());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, refreshed_tuple.get_error());
    FT_ASSERT_STR_EQ("eleven", refreshed_tuple.get<1>().c_str());
    return (1);
}

FT_TEST(test_ft_tuple_move_constructor_preserves_values)
{
    ft_string five_string;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, five_string.initialize("five"));
    duo_tuple source_tuple;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_tuple.initialize());
    source_tuple.get<0>() = 5;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_tuple.get<1>().initialize(five_string));
    duo_tuple moved_tuple;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_tuple.move(source_tuple));

    int moved_value = moved_tuple.get<0>();
    FT_ASSERT_EQ(5, moved_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_tuple.get_error());

    ft_string &moved_string = moved_tuple.get<1>();
    FT_ASSERT_STR_EQ("five", moved_string.c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_tuple.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_tuple.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_tuple.get_error());
    source_tuple.reset();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_tuple.get_error());
    return (1);
}

FT_TEST(test_ft_tuple_move_into_initialized_destination_preserves_source_thread_safety)
{
    ft_tuple<int, int> destination_tuple(1, 2);
    ft_tuple<int, int> source_tuple(7, 9);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_tuple.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_tuple.get_error());
    FT_ASSERT_EQ(FT_FALSE, source_tuple.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_tuple.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_tuple.move(source_tuple));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_tuple.get_error());
    FT_ASSERT_EQ(FT_FALSE, destination_tuple.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_tuple.get_error());
    FT_ASSERT_EQ(7, destination_tuple.get<0>());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_tuple.get_error());
    FT_ASSERT_EQ(9, destination_tuple.get<1>());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_tuple.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_tuple.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_tuple.get_error());
    source_tuple.reset();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_tuple.get_error());
    return (1);
}

FT_TEST(test_ft_tuple_error_queries_follow_lifecycle_contract)
{
    duo_tuple tuple_value;

    g_tuple_get_error_returned = FT_FALSE;
    g_tuple_get_error_result = FT_ERR_SUCCESS;
    g_tuple_get_error_str_returned = FT_FALSE;
    g_tuple_get_error_str_result = ft_nullptr;
    FT_ASSERT_EQ(1, tuple_expect_sigabrt(
        tuple_get_error_uninitialised_operation));
    FT_ASSERT_EQ(FT_FALSE, g_tuple_get_error_returned);
    FT_ASSERT_EQ(1, tuple_expect_sigabrt(
        tuple_get_error_str_uninitialised_operation));
    FT_ASSERT_EQ(FT_FALSE, g_tuple_get_error_str_returned);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tuple_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tuple_value.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tuple_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, tuple_value.get_error());
    FT_ASSERT_EQ(0, ft_strcmp(tuple_value.get_error_str(),
        ft_strerror(FT_ERR_SUCCESS)));
    return (1);
}
