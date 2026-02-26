#include "../test_internal.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CMA/CMA.hpp"
#include "test_cpp_class_string_lifecycle_helpers.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_string_operator_assign_char_initializes_uninitialized_destination,
    "ft_string operator=(char) initializes uninitialized destination")
{
    ft_string destination;

    destination = 'x';
    FT_ASSERT(destination == "x");
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_string::last_operation_error());
    return (1);
}

FT_TEST(test_ft_string_operator_assign_cstring_reinitializes_destroyed_destination,
    "ft_string operator=(cstring) reinitializes destroyed destination")
{
    ft_string destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize("seed"));

    (void)destination.destroy();
    destination = "restored";
    FT_ASSERT(destination == "restored");
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_string::last_operation_error());
    return (1);
}

FT_TEST(test_ft_string_operator_assign_nullptr_clears_string,
    "ft_string operator=(nullptr) clears string")
{
    ft_string destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize("abcdef"));

    destination = static_cast<const char *>(ft_nullptr);
    FT_ASSERT(destination == "");
    FT_ASSERT_EQ(0u, destination.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_string::last_operation_error());
    return (1);
}

FT_TEST(test_ft_string_operator_assign_copy_from_destroyed_source_sets_invalid_state,
    "ft_string copy assignment from destroyed source sets invalid state")
{
    ft_string source;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize("abc"));
    ft_string destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize("keep"));

    (void)source.destroy();
    destination = source;
    int32_t assignment_error = ft_string::last_operation_error();
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, assignment_error);
    FT_ASSERT(destination == "keep");
    return (1);
}

FT_TEST(test_ft_string_operator_assign_move_from_destroyed_source_sets_invalid_state,
    "ft_string move assignment from destroyed source sets invalid state")
{
    ft_string source;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize("abc"));
    ft_string destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize("keep"));

    (void)source.destroy();
    destination = static_cast<ft_string &&>(source);
    int32_t assignment_error = ft_string::last_operation_error();
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, assignment_error);
    FT_ASSERT(destination == "keep");
    return (1);
}

FT_TEST(test_ft_string_operator_plus_equal_nullptr_sets_invalid_argument,
    "ft_string operator+= nullptr sets invalid argument and preserves content")
{
    ft_string value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize("abc"));

    ft_string value_copy(value);
    value += static_cast<const char *>(ft_nullptr);
    int32_t append_error = ft_string::last_operation_error();
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, append_error);
    FT_ASSERT_EQ(value_copy, value);
    return (1);
}

static int string_call_plus_equal_destroyed_source(void)
{
    ft_string value;
    if (value.initialize("abc") != FT_ERR_SUCCESS)
        return (0);
    ft_string source;
    if (source.initialize("xyz") != FT_ERR_SUCCESS)
        return (0);

    (void)source.destroy();
    value += source;
    return (1);
}

FT_TEST(test_ft_string_operator_plus_equal_destroyed_source_sets_invalid_state,
    "ft_string operator+= destroyed source sets invalid state")
{
    FT_ASSERT_EQ(1, string_expect_sigabrt_signal_handler(
            string_call_plus_equal_destroyed_source));
    return (1);
}

FT_TEST(test_ft_string_operator_index_valid_sets_success,
    "ft_string operator[] valid index returns char and sets success")
{
    ft_string value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize("hello"));

    FT_ASSERT_EQ('e', value[1]);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_string::last_operation_error());
    return (1);
}

FT_TEST(test_ft_string_operator_index_out_of_range_sets_error,
    "ft_string operator[] out of range sets error")
{
    ft_string value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize("hello"));

    FT_ASSERT_EQ('\0', value[100]);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_string::last_operation_error());
    return (1);
}

FT_TEST(test_ft_string_global_char_pointer_equals_operator_works,
    "ft_string global char pointer equals operator compares correctly")
{
    ft_string value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize("token"));

    FT_ASSERT("token" == value);
    FT_ASSERT("other" != value);
    return (1);
}

FT_TEST(test_ft_string_operator_plus_char_left_and_right,
    "ft_string operator+ supports char on both sides")
{
    ft_string center;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, center.initialize("core"));
    ft_string left_result;
    ft_string right_result;

    left_result = '[' + center;
    right_result = center + ']';
    FT_ASSERT(left_result == "[core");
    FT_ASSERT(right_result == "core]");
    return (1);
}

FT_TEST(test_ft_string_operator_plus_chain_failure_from_char_prefix_propagates,
    "ft_string chain from char prefix propagates failure state")
{
    ft_string destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize("seed"));
    ft_string source;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize("abcdefghijklmnop"));

    cma_set_alloc_limit(1);
    destination = '!' + source;
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_string::last_operation_error());
    return (1);
}
