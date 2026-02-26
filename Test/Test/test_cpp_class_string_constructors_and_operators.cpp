#include "../test_internal.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CMA/CMA.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_string_constructor_from_cstring_initializes,
    "ft_string cstring constructor initializes content")
{
    ft_string string_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.initialize("hello"));

    FT_ASSERT_EQ(5u, string_value.size());
    FT_ASSERT(string_value == "hello");
    return (1);
}

FT_TEST(test_ft_string_copy_constructor_copies_content,
    "ft_string copy constructor copies data")
{
    ft_string source;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize("alpha"));
    ft_string copy(source);

    FT_ASSERT(copy == "alpha");
    FT_ASSERT(source == "alpha");
    return (1);
}

FT_TEST(test_ft_string_move_constructor_moves_content,
    "ft_string move constructor transfers content")
{
    ft_string source;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize("bravo"));
    ft_string moved(static_cast<ft_string &&>(source));

    FT_ASSERT(moved == "bravo");
    FT_ASSERT_EQ(0u, source.size());
    return (1);
}

FT_TEST(test_ft_string_copy_assignment_destination_uninitialized,
    "ft_string copy assignment initializes uninitialized destination")
{
    ft_string source;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize("copy"));
    ft_string destination;

    destination = source;
    FT_ASSERT(destination == "copy");
    return (1);
}

FT_TEST(test_ft_string_copy_assignment_destination_initialized,
    "ft_string copy assignment destroys and replaces initialized destination")
{
    ft_string source;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize("new"));
    ft_string destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize("old"));

    destination = source;
    FT_ASSERT(destination == "new");
    return (1);
}

FT_TEST(test_ft_string_copy_assignment_destination_destroyed,
    "ft_string copy assignment allows destroyed destination")
{
    ft_string source;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize("after_destroy"));
    ft_string destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize("temp"));

    (void)destination.destroy();
    destination = source;
    FT_ASSERT(destination == "after_destroy");
    return (1);
}

FT_TEST(test_ft_string_move_assignment_destination_initialized,
    "ft_string move assignment replaces initialized destination")
{
    ft_string source;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize("move_me"));
    ft_string destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize("before"));

    destination = static_cast<ft_string &&>(source);
    FT_ASSERT(destination == "move_me");
    FT_ASSERT_EQ(0u, source.size());
    return (1);
}

FT_TEST(test_ft_string_operator_plus_equal_variants,
    "ft_string operator+= works for char, cstring, and ft_string")
{
    ft_string base;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, base.initialize("a"));
    ft_string suffix;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, suffix.initialize("d"));

    base += 'b';
    base += "c";
    base += suffix;
    FT_ASSERT(base == "abcd");
    return (1);
}

FT_TEST(test_ft_string_operator_plus_variants,
    "ft_string operator+ works for string and char combinations")
{
    ft_string left;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, left.initialize("left"));
    ft_string right;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right.initialize("right"));
    ft_string combined;
    ft_string prefixed;
    ft_string suffixed;

    combined = left + right;
    prefixed = '!' + left;
    suffixed = right + '?';
    FT_ASSERT(combined == "leftright");
    FT_ASSERT(prefixed == "!left");
    FT_ASSERT(suffixed == "right?");
    return (1);
}

FT_TEST(test_ft_string_chain_error_propagates_to_assignment,
    "ft_string c = a + b propagates temporary operation error")
{
    ft_string destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize("seed"));
    ft_string left;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, left.initialize("abcd"));
    ft_string right;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right.initialize("efgh"));
    ft_string_proxy proxy_result;

    cma_set_alloc_limit(1);
    proxy_result = left + right;
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, proxy_result.get_error());
    destination = proxy_result;
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_string::last_operation_error());
    return (1);
}

FT_TEST(test_ft_string_chain_error_does_not_stick_after_success,
    "ft_string successful follow-up clears prior chain error")
{
    ft_string destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize("seed"));
    ft_string left;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, left.initialize("abcd"));
    ft_string right;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right.initialize("efgh"));
    ft_string_proxy proxy_result;

    cma_set_alloc_limit(1);
    proxy_result = left + right;
    cma_set_alloc_limit(0);
        FT_ASSERT_EQ(FT_ERR_NO_MEMORY, proxy_result.get_error());
    destination = proxy_result;
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_string::last_operation_error());
    destination = "ok";
    FT_ASSERT(destination == "ok");
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_string::last_operation_error());
    return (1);
}
