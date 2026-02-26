#include "../test_internal.hpp"
#include "../../Template/pair.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../System_utils/test_runner.hpp"
#include <type_traits>
#include <utility>

#ifndef LIBFT_TEST_BUILD
#endif

static_assert(std::is_same<Pair<int, ft_string>, decltype(ft_make_pair(1, ft_string()))>::value,
        "ft_make_pair deduces Pair<int, ft_string>");
static_assert(std::is_same<Pair<int, const char *>, decltype(ft_make_pair(1, "literal"))>::value,
        "ft_make_pair preserves pointer value types");

FT_TEST(test_template_pair_constructs_from_lvalue, "Pair copies values from lvalue arguments")
{
    ft_string value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize("stored"));
    Pair<int, ft_string> pair(42, value);

    FT_ASSERT_EQ(42, pair.key);
    FT_ASSERT_STR_EQ("stored", value.c_str());
    FT_ASSERT_STR_EQ("stored", pair.value.c_str());
    return (1);
}

FT_TEST(test_template_pair_constructs_from_rvalue, "Pair move-constructs value from rvalues")
{
    ft_string value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize("moved"));
    Pair<int, ft_string> pair(7, std::move(value));

    FT_ASSERT_EQ(7, pair.key);
    FT_ASSERT_STR_EQ("moved", pair.value.c_str());
    FT_ASSERT(value.empty());
    return (1);
}

FT_TEST(test_template_pair_make_pair_with_lvalue, "ft_make_pair copies when provided lvalues")
{
    ft_string text;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, text.initialize("example"));
    Pair<int, ft_string> pair = ft_make_pair(3, text);

    FT_ASSERT_EQ(3, pair.key);
    FT_ASSERT_STR_EQ("example", text.c_str());
    FT_ASSERT_STR_EQ("example", pair.value.c_str());
    return (1);
}

FT_TEST(test_template_pair_make_pair_with_rvalue, "ft_make_pair accepts rvalue values")
{
    ft_string temporary;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, temporary.initialize("temporary"));
    Pair<int, ft_string> pair = ft_make_pair(9, temporary);

    FT_ASSERT_EQ(9, pair.key);
    FT_ASSERT_STR_EQ("temporary", pair.value.c_str());
    return (1);
}

FT_TEST(test_template_pair_make_pair_with_pointer, "ft_make_pair stores pointer values without modification")
{
    const char *literal;
    Pair<int, const char *> pair;

    literal = "pointer";
    pair = ft_make_pair(5, literal);
    FT_ASSERT_EQ(5, pair.key);
    FT_ASSERT(pair.value == literal);
    return (1);
}
