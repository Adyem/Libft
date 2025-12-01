#include "../../Math/math.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_math_swap_exchanges_values, "math_swap exchanges pointed integers")
{
    int first_number;
    int second_number;

    first_number = 3;
    second_number = 9;
    math_swap(&first_number, &second_number);
    FT_ASSERT_EQ(9, first_number);
    FT_ASSERT_EQ(3, second_number);
    return (1);
}

FT_TEST(test_math_swap_null_pointer_safe, "math_swap ignores null pointers")
{
    int value;

    value = 11;
    math_swap(static_cast<int *>(ft_nullptr), &value);
    FT_ASSERT_EQ(11, value);
    math_swap(&value, static_cast<int *>(ft_nullptr));
    FT_ASSERT_EQ(11, value);
    return (1);
}

FT_TEST(test_math_swap_same_pointer_noop, "math_swap leaves value unchanged when both pointers match")
{
    int value;

    value = 7;
    math_swap(&value, &value);
    FT_ASSERT_EQ(7, value);
    return (1);
}
