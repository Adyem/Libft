#include "../test_internal.hpp"
#include "../../CPP_class/class_big_number.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_big_number_operation_stack_ctor_records_success,
        "ft_big_number ctor records success in operation stack")
{
    ft_big_number::pop_operation_errors();
    ft_big_number number_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(0, ft_big_number::operation_error_index());
    return (1);
}

FT_TEST(test_ft_big_number_operation_stack_append_digit_success,
        "ft_big_number append digit records success")
{
    ft_big_number::pop_operation_errors();
    ft_big_number number_value;

    number_value.append_digit('3');
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_big_number::operation_error_at(1));
    FT_ASSERT_EQ(0, ft_big_number::operation_error_index());
    return (1);
}

FT_TEST(test_ft_big_number_operation_stack_append_digit_invalid,
        "ft_big_number append invalid digit records error")
{
    ft_big_number::pop_operation_errors();
    ft_big_number number_value;

    number_value.append_digit('X');
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(1, ft_big_number::operation_error_index());
    return (1);
}

FT_TEST(test_ft_big_number_operation_stack_assign_success,
        "ft_big_number assign records success")
{
    ft_big_number::pop_operation_errors();
    ft_big_number number_value;

    number_value.assign("42");
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(0, ft_big_number::operation_error_index());
    return (1);
}

FT_TEST(test_ft_big_number_operation_stack_assign_null_invalid,
        "ft_big_number assign null pointer records error")
{
    ft_big_number::pop_operation_errors();
    ft_big_number number_value;

    number_value.assign(static_cast<const char *>(ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_big_number::last_operation_error());
    return (1);
}

FT_TEST(test_ft_big_number_operation_stack_assign_base_success,
        "ft_big_number assign_base records success")
{
    ft_big_number::pop_operation_errors();
    ft_big_number number_value;

    number_value.assign_base("1f", 16);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(0, ft_big_number::operation_error_index());
    return (1);
}

FT_TEST(test_ft_big_number_operation_stack_assign_base_invalid_base,
        "ft_big_number assign_base invalid base records error")
{
    ft_big_number::pop_operation_errors();
    ft_big_number number_value;

    number_value.assign_base("10", 1);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_big_number::last_operation_error());
    return (1);
}

FT_TEST(test_ft_big_number_operation_stack_assign_base_invalid_digit,
        "ft_big_number assign_base invalid digit records error")
{
    ft_big_number::pop_operation_errors();
    ft_big_number number_value;

    number_value.assign_base("1G", 16);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_big_number::last_operation_error());
    return (1);
}

FT_TEST(test_ft_big_number_operation_stack_out_of_range_index_success,
        "ft_big_number operation_error_at out of range returns success")
{
    ft_big_number::pop_operation_errors();
    ft_big_number number_value;

    number_value.append_digit('2');
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_big_number::operation_error_at(4));
    return (1);
}

FT_TEST(test_ft_big_number_operation_stack_pop_oldest_returns_oldest,
        "ft_big_number pop oldest returns earliest error")
{
    ft_big_number::pop_operation_errors();
    ft_big_number number_value;

    number_value.append_digit('X');
    number_value.append_digit('5');
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_big_number::pop_oldest_operation_error());
    FT_ASSERT_EQ(0, ft_big_number::operation_error_index());
    return (1);
}
