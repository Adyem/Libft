#include "../test_internal.hpp"
#include "../../Modules/CPP_class/class_string.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_cpp_class_string_assigns_from_literal)
{
    ft_string target;

    ft_string &result = (target = "hello");
    FT_ASSERT(&result == &target);
    FT_ASSERT(target == "hello");
    return (1);
}

FT_TEST(test_cpp_class_string_assigns_from_pointer)
{
    ft_string target;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, target.initialize("prefix"));
    const char *source = "world";

    target = source;
    FT_ASSERT(target == source);
    FT_ASSERT(target.c_str() != source);
    return (1);
}

FT_TEST(test_cpp_class_string_plus_equals_appends_string)
{
    ft_string base;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, base.initialize("Hello"));
    ft_string suffix;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, suffix.initialize(" World"));

    ft_string &result = (base += suffix);
    FT_ASSERT(&result == &base);
    FT_ASSERT(base == "Hello World");
    FT_ASSERT(suffix == " World");
    return (1);
}

FT_TEST(test_cpp_class_string_char_prefix_concatenation)
{
    ft_string base;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, base.initialize("core"));
    ft_string prefixed;

    prefixed = '!' + base;
    FT_ASSERT(prefixed == "!core");
    FT_ASSERT(base == "core");
    return (1);
}
