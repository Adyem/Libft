#include "../../Basic/basic.hpp"
#include "../../System_utils/test_runner.hpp"

constexpr bool g_compile_time_is_constant = ft_is_constant_evaluated();

FT_TEST(test_repeat_byte_replicates_input, "ft_detail::repeat_byte copies the byte across the word")
{
    size_t expected;
    size_t index;

    expected = 0;
    index = 0;
    while (index < sizeof(size_t))
    {
        expected <<= 8;
        expected |= static_cast<size_t>(0xAB);
        index++;
    }
    FT_ASSERT_EQ(expected, ft_detail::repeat_byte(0xAB));
    return (1);
}

FT_TEST(test_repeat_byte_zero_input_produces_zero, "ft_detail::repeat_byte preserves zero input")
{
    FT_ASSERT_EQ(static_cast<size_t>(0), ft_detail::repeat_byte(0));
    return (1);
}

FT_TEST(test_has_zero_detects_zero_byte, "ft_detail::has_zero detects when any byte is zero")
{
    size_t pattern;
    size_t zero_pattern;

    pattern = ft_detail::repeat_byte(0x7F);
    zero_pattern = pattern & ~static_cast<size_t>(0xFF);
    FT_ASSERT_EQ(false, ft_detail::has_zero(pattern));
    FT_ASSERT_EQ(true, ft_detail::has_zero(zero_pattern));
    return (1);
}

FT_TEST(test_is_constant_evaluated_runtime_false, "ft_is_constant_evaluated returns false at runtime")
{
    bool runtime_result;

    runtime_result = ft_is_constant_evaluated();
    FT_ASSERT_EQ(false, runtime_result);
    return (1);
}

FT_TEST(test_is_constant_evaluated_compile_time_flag_consistent, "ft_is_constant_evaluated compile-time behavior matches expectations")
{
    if (g_compile_time_is_constant)
    {
        FT_ASSERT_EQ(true, g_compile_time_is_constant);
        FT_ASSERT_EQ(false, ft_is_constant_evaluated());
    }
    else
    {
        FT_ASSERT_EQ(false, g_compile_time_is_constant);
        FT_ASSERT_EQ(false, ft_is_constant_evaluated());
    }
    return (1);
}
