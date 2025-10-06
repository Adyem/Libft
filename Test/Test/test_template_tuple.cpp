#include "../../Template/tuple.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CPP_class/class_string_class.hpp"
#include <utility>

FT_TEST(test_ft_tuple_construction_and_get, "ft_tuple stores provided values and exposes them by index and type")
{
    ft_tuple<int, ft_string, double> tuple_instance(7, ft_string("value"), 3.5);

    int first_value = tuple_instance.get<0>();
    FT_ASSERT_EQ(7, first_value);
    FT_ASSERT_EQ(ER_SUCCESS, tuple_instance.get_error());

    ft_string &second_value = tuple_instance.get<1>();
    FT_ASSERT(second_value == "value");
    FT_ASSERT_EQ(ER_SUCCESS, tuple_instance.get_error());

    double third_value = tuple_instance.get<2>();
    FT_ASSERT_EQ(3.5, third_value);
    FT_ASSERT_EQ(ER_SUCCESS, tuple_instance.get_error());

    double type_lookup = tuple_instance.get<double>();
    FT_ASSERT_EQ(3.5, type_lookup);
    FT_ASSERT_EQ(ER_SUCCESS, tuple_instance.get_error());
    return (1);
}

FT_TEST(test_ft_tuple_reset_and_error_reporting, "ft_tuple signals bad access after reset and recovers on new construction")
{
    ft_tuple<int, ft_string> tuple_instance(42, ft_string("answer"));

    tuple_instance.reset();
    FT_ASSERT_EQ(ER_SUCCESS, tuple_instance.get_error());

    int missing_value = tuple_instance.get<0>();
    FT_ASSERT_EQ(0, missing_value);
    FT_ASSERT_EQ(TUPLE_BAD_ACCESS, tuple_instance.get_error());

    ft_string &missing_string = tuple_instance.get<1>();
    FT_ASSERT(missing_string == "");
    FT_ASSERT_EQ(TUPLE_BAD_ACCESS, tuple_instance.get_error());

    ft_tuple<int, ft_string> refreshed_tuple(11, ft_string("eleven"));
    tuple_instance = std::move(refreshed_tuple);
    FT_ASSERT_EQ(ER_SUCCESS, tuple_instance.get_error());
    FT_ASSERT_EQ(11, tuple_instance.get<0>());
    FT_ASSERT_EQ(ER_SUCCESS, tuple_instance.get_error());
    return (1);
}

FT_TEST(test_ft_tuple_move_semantics, "ft_tuple move constructor releases original storage and retains values")
{
    ft_tuple<int, ft_string> original_tuple(5, ft_string("five"));

    ft_tuple<int, ft_string> moved_tuple(std::move(original_tuple));

    int moved_value = moved_tuple.get<0>();
    FT_ASSERT_EQ(5, moved_value);
    FT_ASSERT_EQ(ER_SUCCESS, moved_tuple.get_error());

    ft_string &moved_string = moved_tuple.get<1>();
    FT_ASSERT(moved_string == "five");
    FT_ASSERT_EQ(ER_SUCCESS, moved_tuple.get_error());

    int default_value = original_tuple.get<0>();
    FT_ASSERT_EQ(0, default_value);
    FT_ASSERT_EQ(TUPLE_BAD_ACCESS, original_tuple.get_error());

    ft_tuple<int, ft_string> assigned_tuple(1, ft_string("one"));
    assigned_tuple = std::move(moved_tuple);

    FT_ASSERT_EQ(ER_SUCCESS, moved_tuple.get_error());
    int assigned_value = assigned_tuple.get<0>();
    FT_ASSERT_EQ(5, assigned_value);
    FT_ASSERT_EQ(ER_SUCCESS, assigned_tuple.get_error());
    return (1);
}
