#include "../../Template/math.hpp"
#include "../../System_utils/test_runner.hpp"

static_assert(ft_max(1, 2) == 2, "ft_max constexpr check");
static_assert(ft_min(1, 2) == 1, "ft_min constexpr check");
static_assert(is_single_convertible_to_size_t<unsigned int>::value, "unsigned int convertible to size_t");
static_assert(!is_single_convertible_to_size_t<void *>::value, "void pointer not convertible to size_t");
static_assert(!is_single_convertible_to_size_t<int, int>::value, "multiple parameters fail convertibility");

static bool compare_absolute_value(int left, int right)
{
    int left_absolute;
    int right_absolute;

    left_absolute = left;
    if (left < 0)
        left_absolute = -left;
    right_absolute = right;
    if (right < 0)
        right_absolute = -right;
    if (left_absolute < right_absolute)
        return (true);
    return (false);
}

FT_TEST(test_template_math_ft_max_basic, "ft_max returns the greater value")
{
    double mixed_result;

    FT_ASSERT_EQ(7, ft_max(7, 3));
    FT_ASSERT_EQ(5, ft_max(2, 5));
    mixed_result = ft_max(3, 4.5);
    FT_ASSERT(mixed_result == 4.5);
    return (1);
}

FT_TEST(test_template_math_ft_min_basic, "ft_min returns the smaller value")
{
    double mixed_result;

    FT_ASSERT_EQ(3, ft_min(7, 3));
    FT_ASSERT_EQ(-2, ft_min(-2, 5));
    mixed_result = ft_min(3, 4.5);
    FT_ASSERT(mixed_result == 3.0);
    return (1);
}

FT_TEST(test_template_math_ft_max_with_comparator, "ft_max honors custom comparators")
{
    FT_ASSERT_EQ(-7, ft_max(-7, 4, compare_absolute_value));
    FT_ASSERT_EQ(10, ft_max(10, -3, compare_absolute_value));
    return (1);
}

FT_TEST(test_template_math_ft_min_with_comparator, "ft_min honors custom comparators")
{
    FT_ASSERT_EQ(4, ft_min(-7, 4, compare_absolute_value));
    FT_ASSERT_EQ(-3, ft_min(10, -3, compare_absolute_value));
    return (1);
}

FT_TEST(test_template_math_is_single_convertible_runtime, "is_single_convertible_to_size_t matches compile-time expectations")
{
    bool pointer_convertible;
    bool multiple_parameter_convertible;

    pointer_convertible = is_single_convertible_to_size_t<const char *>::value;
    multiple_parameter_convertible = is_single_convertible_to_size_t<int, long>::value;
    FT_ASSERT(is_single_convertible_to_size_t<size_t>::value);
    FT_ASSERT(is_single_convertible_to_size_t<unsigned short>::value);
    FT_ASSERT(!pointer_convertible);
    FT_ASSERT(!multiple_parameter_convertible);
    return (1);
}
