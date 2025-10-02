#include "../../Template/pair.hpp"
#include "../../CPP_class/class_string_class.hpp"
#include "../../System_utils/test_runner.hpp"
#include <type_traits>

static_assert(std::is_same<Pair<int, ft_string>, decltype(ft_make_pair(1, ft_string("value")))>::value,
        "ft_make_pair deduces Pair<int, ft_string>");
static_assert(std::is_same<Pair<int, const char *>, decltype(ft_make_pair(1, "literal"))>::value,
        "ft_make_pair preserves pointer value types");

FT_TEST(test_template_pair_constructs_from_lvalue, "Pair copies values from lvalue arguments")
{
    ft_string value("stored");
    Pair<int, ft_string> pair(42, value);

    FT_ASSERT_EQ(42, pair.key);
    FT_ASSERT(pair.value == value);
    FT_ASSERT(value == ft_string("stored"));
    return (1);
}

FT_TEST(test_template_pair_constructs_from_rvalue, "Pair move-constructs value from rvalues")
{
    ft_string value("moved");
    Pair<int, ft_string> pair(7, ft_move(value));

    FT_ASSERT_EQ(7, pair.key);
    FT_ASSERT(pair.value == ft_string("moved"));
    FT_ASSERT(value.empty());
    return (1);
}

FT_TEST(test_template_pair_make_pair_with_lvalue, "ft_make_pair copies when provided lvalues")
{
    ft_string text("example");
    Pair<int, ft_string> pair = ft_make_pair(3, text);

    FT_ASSERT_EQ(3, pair.key);
    FT_ASSERT(pair.value == text);
    FT_ASSERT(text == ft_string("example"));
    return (1);
}

FT_TEST(test_template_pair_make_pair_with_rvalue, "ft_make_pair accepts rvalue values")
{
    Pair<int, ft_string> pair = ft_make_pair(9, ft_string("temporary"));

    FT_ASSERT_EQ(9, pair.key);
    FT_ASSERT(pair.value == ft_string("temporary"));
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
