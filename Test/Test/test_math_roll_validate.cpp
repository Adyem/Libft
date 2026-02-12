#include "../test_internal.hpp"
#include "../../Math/math_internal.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_math_roll_validate_rejects_punctuation, "math_roll_validate rejects punctuation in argument scan")
{
    char expression[] = "!2+3";
    int validation_result;

    validation_result = math_roll_validate(expression);
    FT_ASSERT_EQ(1, validation_result);
    return (1);
}

