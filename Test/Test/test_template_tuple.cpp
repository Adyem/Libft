#include "../test_internal.hpp"
#include "../../Template/tuple.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CPP_class/class_string.hpp"


#ifndef LIBFT_TEST_BUILD
#endif

using triple_tuple = ft_tuple<int, ft_string, double>;
using duo_tuple = ft_tuple<int, ft_string>;

FT_TEST(test_ft_tuple_construction_and_get, "ft_tuple stores provided values and exposes them by index and type")
{
    ft_string second_string;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second_string.initialize("value"));
    triple_tuple tuple_instance(7, second_string, 3.5);

    int first_value = tuple_instance.get<0>();
    FT_ASSERT_EQ(7, first_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, triple_tuple::last_operation_error());

    ft_string &second_value = tuple_instance.get<1>();
    FT_ASSERT_STR_EQ("value", second_value.c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, triple_tuple::last_operation_error());

    double third_value = tuple_instance.get<2>();
    FT_ASSERT_EQ(3.5, third_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, triple_tuple::last_operation_error());

    double type_lookup = tuple_instance.get<double>();
    FT_ASSERT_EQ(3.5, type_lookup);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, triple_tuple::last_operation_error());
    return (1);
}

FT_TEST(test_ft_tuple_reset_and_error_reporting, "ft_tuple signals bad access after reset and recovers on new construction")
{
    ft_string answer_string;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, answer_string.initialize("answer"));
    duo_tuple tuple_instance(42, answer_string);

    tuple_instance.reset();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duo_tuple::last_operation_error());

    int missing_value = tuple_instance.get<0>();
    FT_ASSERT_EQ(0, missing_value);
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, duo_tuple::last_operation_error());

    ft_string &missing_string = tuple_instance.get<1>();
    FT_ASSERT_STR_EQ("", missing_string.c_str());
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, duo_tuple::last_operation_error());

    ft_string eleven_string;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, eleven_string.initialize("eleven"));
    duo_tuple refreshed_tuple(11, eleven_string);
    tuple_instance.reset();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duo_tuple::last_operation_error());
    FT_ASSERT_EQ(11, refreshed_tuple.get<0>());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duo_tuple::last_operation_error());
    FT_ASSERT_STR_EQ("eleven", refreshed_tuple.get<1>().c_str());
    return (1);
}

FT_TEST(test_ft_tuple_move_semantics, "ft_tuple move constructor releases original storage and retains values")
{
    ft_string five_string;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, five_string.initialize("five"));
    duo_tuple copy_tuple(5, five_string);

    int copied_value = copy_tuple.get<0>();
    FT_ASSERT_EQ(5, copied_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duo_tuple::last_operation_error());

    ft_string &copied_string = copy_tuple.get<1>();
    FT_ASSERT_STR_EQ("five", copied_string.c_str());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duo_tuple::last_operation_error());

    ft_tuple<int, ft_string> empty_tuple;
    int empty_value = empty_tuple.get<0>();
    FT_ASSERT_EQ(0, empty_value);
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, duo_tuple::last_operation_error());
    return (1);
}
