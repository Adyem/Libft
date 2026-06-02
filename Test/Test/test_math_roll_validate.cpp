#include "../test_internal.hpp"
#include "../../Modules/Math/math_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_math_roll_validate_rejects_punctuation)
{
    char expression[] = "!2+3";
    int validation_result;

    validation_result = math_roll_validate(expression);
    FT_ASSERT_EQ(1, validation_result);
    return (1);
}

