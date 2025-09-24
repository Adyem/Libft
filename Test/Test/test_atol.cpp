#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Libft/limits.hpp"
#include "../../CPP_class/class_nullptr.hpp"

FT_TEST(test_atol_overflow_clamps_to_long_max, "ft_atol clamps overflow to FT_LONG_MAX")
{
    ft_string overflow_string;

    overflow_string = ft_to_string(FT_LONG_MAX);
    overflow_string += "9";
    FT_ASSERT_EQ(FT_LONG_MAX, ft_atol(overflow_string.c_str()));
    return (1);
}

FT_TEST(test_atol_underflow_clamps_to_long_min, "ft_atol clamps underflow to FT_LONG_MIN")
{
    ft_string underflow_string;

    underflow_string = ft_to_string(FT_LONG_MIN);
    underflow_string += "9";
    FT_ASSERT_EQ(FT_LONG_MIN, ft_atol(underflow_string.c_str()));
    return (1);
}

FT_TEST(test_atol_null_input, "ft_atol null input returns zero")
{
    FT_ASSERT_EQ(0L, ft_atol(ft_nullptr));
    return (1);
}
