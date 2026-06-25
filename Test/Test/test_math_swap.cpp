#include "../test_internal.hpp"
#include "../../Modules/Math/math.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Math/math_interval.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_math_swap_exchanges_values)
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

FT_TEST(test_math_swap_null_pointer_safe)
{
    int value;

    value = 11;
    math_swap(static_cast<int *>(ft_nullptr), &value);
    FT_ASSERT_EQ(11, value);
    math_swap(&value, static_cast<int *>(ft_nullptr));
    FT_ASSERT_EQ(11, value);
    return (1);
}

FT_TEST(test_math_swap_same_pointer_noop)
{
    int value;

    value = 7;
    math_swap(&value, &value);
    FT_ASSERT_EQ(7, value);
    return (1);
}
