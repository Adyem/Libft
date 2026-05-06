#include "../test_internal.hpp"
#include "../../Modules/Math/math_internal.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/CPP_class/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_math_check_string_number_accepts_unsigned_input)
{
    char number_string[] = "12345";

    FT_ASSERT_EQ(1, math_check_string_number(number_string));
    return (1);
}

FT_TEST(test_math_check_string_number_accepts_signed_input)
{
    char number_string[] = "-678";

    FT_ASSERT_EQ(1, math_check_string_number(number_string));
    return (1);
}

FT_TEST(test_math_check_string_number_rejects_invalid_characters)
{
    char number_string[] = "42a";

    FT_ASSERT_EQ(0, math_check_string_number(number_string));
    return (1);
}

FT_TEST(test_math_check_string_number_rejects_sign_without_digits)
{
    char number_string[] = "-";

    FT_ASSERT_EQ(0, math_check_string_number(number_string));
    return (1);
}

FT_TEST(test_math_roll_convert_previous_parses_adjacent_number)
{
    char expression[] = "12d6";
    int index = 1;
    int error = 0;
    int result;

    result = math_roll_convert_previous(expression, &index, &error);
    FT_ASSERT_EQ(12, result);
    FT_ASSERT_EQ(0, error);
    FT_ASSERT_EQ(0, index);
    return (1);
}

FT_TEST(test_math_roll_convert_previous_handles_unary_negative)
{
    char expression[] = "-3d4";
    int index = 1;
    int error = 0;
    int result;

    result = math_roll_convert_previous(expression, &index, &error);
    FT_ASSERT_EQ(-3, result);
    FT_ASSERT_EQ(0, error);
    FT_ASSERT_EQ(0, index);
    return (1);
}

FT_TEST(test_math_roll_convert_previous_skips_non_digit_start)
{
    char expression[] = "*5+3";
    int index = 0;
    int error = 0;
    int result;

    result = math_roll_convert_previous(expression, &index, &error);
    FT_ASSERT_EQ(5, result);
    FT_ASSERT_EQ(0, error);
    FT_ASSERT_EQ(1, index);
    return (1);
}

FT_TEST(test_math_roll_convert_previous_sets_error_on_overflow)
{
    char expression[] = "2147483648d6";
    int index = 9;
    int error = 0;
    int result;

    result = math_roll_convert_previous(expression, &index, &error);
    FT_ASSERT_EQ(0, result);
    FT_ASSERT_EQ(1, error);
    FT_ASSERT_EQ(0, index);
    return (1);
}

FT_TEST(test_math_roll_convert_next_parses_following_number)
{
    char expression[] = "1d20+3";
    int error = 0;
    int result;

    result = math_roll_convert_next(expression, 2, &error);
    FT_ASSERT_EQ(20, result);
    FT_ASSERT_EQ(0, error);
    return (1);
}

FT_TEST(test_math_roll_convert_next_sets_error_on_overflow)
{
    char expression[] = "d2147483648";
    int error = 0;
    int result;

    result = math_roll_convert_next(expression, 1, &error);
    FT_ASSERT_EQ(0, result);
    FT_ASSERT_EQ(1, error);
    return (1);
}

FT_TEST(test_math_roll_itoa_appends_results_sequentially)
{
    char buffer[32];
    int index;
    int position;

    index = 0;
    position = 0;
    while (position < 32)
    {
        buffer[position] = '\0';
        position++;
    }
    FT_ASSERT_EQ(0, math_roll_itoa(42, &index, buffer));
    FT_ASSERT_EQ(2, index);
    FT_ASSERT_EQ('4', buffer[0]);
    FT_ASSERT_EQ('2', buffer[1]);
    FT_ASSERT_EQ(0, math_roll_itoa(-7, &index, buffer));
    FT_ASSERT_EQ(4, index);
    FT_ASSERT_EQ('-', buffer[2]);
    FT_ASSERT_EQ('7', buffer[3]);
    return (1);
}

FT_TEST(test_math_free_parse_releases_allocated_strings)
{
    char **strings;
    ft_size_t free_count_before;
    ft_size_t free_count_after;

    strings = static_cast<char **>(cma_malloc(3 * sizeof(char *)));
    if (!strings)
        return (0);
    strings[0] = ft_nullptr;
    strings[1] = ft_nullptr;
    strings[2] = ft_nullptr;
    strings[0] = static_cast<char *>(cma_malloc(3));
    if (!strings[0])
    {
        cma_free(strings);
        return (0);
    }
    strings[0][0] = '1';
    strings[0][1] = '0';
    strings[0][2] = '\0';
    strings[1] = static_cast<char *>(cma_malloc(3));
    if (!strings[1])
    {
        cma_free(strings[0]);
        cma_free(strings);
        return (0);
    }
    strings[1][0] = '-';
    strings[1][1] = '5';
    strings[1][2] = '\0';
    cma_get_stats(ft_nullptr, &free_count_before);
    math_free_parse(strings);
    cma_get_stats(ft_nullptr, &free_count_after);
    FT_ASSERT_EQ(free_count_before + 3, free_count_after);
    return (1);
}
