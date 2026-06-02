#include "../test_internal.hpp"
#include "../../Modules/Math/math_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_math_roll_check_number_next_accepts_digit)
{
    char expression[] = "1+2";

    FT_ASSERT_EQ(0, math_roll_check_number_next(expression, 0));
    return (1);
}

FT_TEST(test_math_roll_check_number_next_accepts_signed_digit)
{
    char expression[] = "2+-3";

    FT_ASSERT_EQ(0, math_roll_check_number_next(expression, 1));
    return (1);
}

FT_TEST(test_math_roll_check_number_next_rejects_invalid_character)
{
    char expression[] = "3+a";

    FT_ASSERT_EQ(1, math_roll_check_number_next(expression, 1));
    return (1);
}

FT_TEST(test_math_roll_check_number_next_rejects_sign_without_digit)
{
    char expression[] = "4+-";

    FT_ASSERT_EQ(1, math_roll_check_number_next(expression, 1));
    return (1);
}

FT_TEST(test_math_roll_check_number_previous_accepts_digit)
{
    char expression[] = "1d6";

    FT_ASSERT_EQ(0, math_roll_check_number_previous(expression, 1));
    return (1);
}

FT_TEST(test_math_roll_check_number_previous_rejects_non_digit)
{
    char expression[] = "(1";

    FT_ASSERT_EQ(1, math_roll_check_number_previous(expression, 1));
    return (1);
}

FT_TEST(test_math_roll_check_character_accepts_supported_tokens)
{
    FT_ASSERT_EQ(0, math_roll_check_character('+'));
    FT_ASSERT_EQ(0, math_roll_check_character('-'));
    FT_ASSERT_EQ(0, math_roll_check_character('*'));
    FT_ASSERT_EQ(0, math_roll_check_character('/'));
    FT_ASSERT_EQ(0, math_roll_check_character('\0'));
    return (1);
}

FT_TEST(test_math_roll_check_character_rejects_other_tokens)
{
    FT_ASSERT_EQ(1, math_roll_check_character('d'));
    FT_ASSERT_EQ(1, math_roll_check_character('('));
    FT_ASSERT_EQ(1, math_roll_check_character('x'));
    FT_ASSERT_EQ(1, math_roll_check_character('1'));
    return (1);
}
