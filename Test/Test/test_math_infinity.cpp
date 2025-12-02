#include "../../Math/math.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_math_infinity_positive, "math_infinity returns positive infinity")
{
    double result;

    result = math_infinity();
    FT_ASSERT(math_isinf(result));
    FT_ASSERT(result > 0.0);
    return (1);
}

FT_TEST(test_math_negative_infinity_negative, "math_negative_infinity returns negative infinity")
{
    double result;

    result = math_negative_infinity();
    FT_ASSERT(math_isinf(result));
    FT_ASSERT(result < 0.0);
    return (1);
}

FT_TEST(test_math_isinf_rejects_non_infinite, "math_isinf ignores finite and NaN values")
{
    FT_ASSERT_EQ(0, math_isinf(0.0));
    FT_ASSERT_EQ(0, math_isinf(math_nan()));
    FT_ASSERT_EQ(0, math_isinf(123.5));
    return (1);
}
