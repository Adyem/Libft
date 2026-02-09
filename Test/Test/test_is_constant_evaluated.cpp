#include "../../Basic/basic.hpp"
#include "../../System_utils/test_runner.hpp"

static constexpr bool g_compile_time_result = ft_is_constant_evaluated();
static_assert(g_compile_time_result, "ft_is_constant_evaluated should return true during constant evaluation");

static size_t build_word_from_bytes(const unsigned char *bytes)
{
    size_t index;
    size_t value;

    index = 0;
    value = 0;
    while (index < sizeof(size_t))
    {
        value <<= 8;
        value |= static_cast<size_t>(bytes[index]);
        index++;
    }
    return (value);
}

FT_TEST(test_ft_is_constant_evaluated_compile_time_true,
        "ft_is_constant_evaluated reports true in constexpr contexts")
{
    constexpr bool compile_time_value = ft_is_constant_evaluated();

    FT_ASSERT_EQ(true, compile_time_value);
    return (1);
}

FT_TEST(test_ft_is_constant_evaluated_runtime_false,
        "ft_is_constant_evaluated reports false at runtime")
{
    bool runtime_value;

    runtime_value = ft_is_constant_evaluated();
    FT_ASSERT_EQ(false, runtime_value);
    return (1);
}

FT_TEST(test_ft_detail_repeat_byte_spreads_pattern,
        "ft_detail::repeat_byte duplicates byte patterns across a word")
{
    size_t expected_value;
    size_t computed_value;
    size_t index;

    expected_value = 0;
    index = 0;
    while (index < sizeof(size_t))
    {
        expected_value <<= 8;
        expected_value |= 0xAB;
        index++;
    }
    computed_value = ft_detail::repeat_byte(0xAB);
    FT_ASSERT_EQ(expected_value, computed_value);
    return (1);
}

FT_TEST(test_ft_detail_has_zero_detects_zero_byte,
        "ft_detail::has_zero detects zero bytes within a word")
{
    unsigned char bytes[sizeof(size_t)];
    size_t index;
    size_t word_value;

    index = 0;
    while (index < sizeof(size_t))
    {
        bytes[index] = 0xFF;
        index++;
    }
    if (sizeof(size_t) > 0)
        bytes[sizeof(size_t) / 2] = 0x00;
    word_value = build_word_from_bytes(bytes);
    FT_ASSERT_EQ(true, ft_detail::has_zero(word_value));
    return (1);
}

FT_TEST(test_ft_detail_has_zero_without_zero_byte,
        "ft_detail::has_zero returns false when no zero byte present")
{
    size_t non_zero_word;

    non_zero_word = ft_detail::repeat_byte(0x7F);
    FT_ASSERT_EQ(false, ft_detail::has_zero(non_zero_word));
    return (1);
}
