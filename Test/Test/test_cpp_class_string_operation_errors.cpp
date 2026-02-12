#include "../test_internal.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_string_operation_stack_ctor_records_success,
        "ft_string ctor records success in operation stack")
{
    ft_string::pop_operation_errors();
    ft_string string_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_string::last_operation_error());
    FT_ASSERT_EQ(0, ft_string::operation_error_index());
    return (1);
}

FT_TEST(test_ft_string_operation_stack_append_char_success,
        "ft_string append char records success")
{
    ft_string::pop_operation_errors();
    ft_string string_value("abc");

    string_value.append('d');
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_string::last_operation_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_string::operation_error_at(1));
    FT_ASSERT_EQ(0, ft_string::operation_error_index());
    return (1);
}

FT_TEST(test_ft_string_operation_stack_assign_count_success,
        "ft_string assign count records success")
{
    ft_string::pop_operation_errors();
    ft_string string_value;

    string_value.assign(3, 'x');
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_string::last_operation_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_string::last_operation_error());
    FT_ASSERT_EQ(0, ft_string::operation_error_index());
    return (1);
}

FT_TEST(test_ft_string_operation_stack_erase_out_of_range_error,
        "ft_string erase out of range records error")
{
    ft_string::pop_operation_errors();
    ft_string string_value("abc");

    string_value.erase(5, 2);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_string::last_operation_error());
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_string::last_operation_error());
    FT_ASSERT_EQ(1, ft_string::operation_error_index());
    return (1);
}

FT_TEST(test_ft_string_operation_stack_append_zero_length_success,
        "ft_string append zero length records success")
{
    ft_string::pop_operation_errors();
    ft_string string_value("abc");

    string_value.append("ignored", 0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_string::last_operation_error());
    FT_ASSERT_EQ(0, ft_string::operation_error_index());
    return (1);
}

FT_TEST(test_ft_string_operation_stack_assign_cstr_success,
        "ft_string assign cstr records success")
{
    ft_string::pop_operation_errors();
    ft_string string_value;

    string_value.assign("data", 4);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_string::last_operation_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_string::last_operation_error());
    FT_ASSERT_EQ(0, ft_string::operation_error_index());
    return (1);
}

FT_TEST(test_ft_string_operation_stack_out_of_range_index_success,
        "ft_string operation_error_at out of range returns success")
{
    ft_string::pop_operation_errors();
    ft_string string_value("abc");

    string_value.append('q');
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_string::operation_error_at(4));
    return (1);
}

FT_TEST(test_ft_string_operation_stack_error_index_after_success,
        "ft_string operation_error_index skips trailing successes")
{
    ft_string::pop_operation_errors();
    ft_string string_value("abc");

    string_value.erase(7, 1);
    string_value.append('y');
    FT_ASSERT_EQ(2, ft_string::operation_error_index());
    return (1);
}

FT_TEST(test_ft_string_operation_stack_pop_oldest_returns_oldest,
        "ft_string pop oldest returns earliest error")
{
    ft_string::pop_operation_errors();
    ft_string string_value("abc");

    string_value.erase(9, 1);
    string_value.append('x');
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_string::pop_oldest_operation_error());
    FT_ASSERT_EQ(0, ft_string::operation_error_index());
    return (1);
}

FT_TEST(test_ft_string_operation_stack_pop_all_clears,
        "ft_string pop operation errors clears stack")
{
    ft_string::pop_operation_errors();
    ft_string string_value("abc");

    string_value.erase(9, 1);
    string_value.erase(10, 1);
    FT_ASSERT_EQ(1, ft_string::operation_error_index());
    ft_string::pop_operation_errors();
    FT_ASSERT_EQ(0, ft_string::operation_error_index());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_string::last_operation_error());
    return (1);
}
