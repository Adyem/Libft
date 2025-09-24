#include "../../CPP_class/class_string_class.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_ft_string_assigns_from_literal, "ft_string operator= accepts string literal")
{
    ft_string target;

    ft_string &result = (target = "hello");
    FT_ASSERT(&result == &target);
    FT_ASSERT(target == "hello");
    return (1);
}

FT_TEST(test_ft_string_assigns_from_pointer, "ft_string operator= accepts pointer value")
{
    ft_string target("prefix");
    const char *source = "world";

    target = source;
    FT_ASSERT(target == source);
    FT_ASSERT(target.c_str() != source);
    return (1);
}

FT_TEST(test_ft_string_plus_equals_appends_string, "ft_string operator+= appends ft_string")
{
    ft_string base("Hello");
    ft_string suffix(" World");

    ft_string &result = (base += suffix);
    FT_ASSERT(&result == &base);
    FT_ASSERT(base == "Hello World");
    FT_ASSERT(suffix == " World");
    return (1);
}
