#include "../test_internal.hpp"
#include "../../Basic/basic.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_basic_hash_string31_null_input)
{
    FT_ASSERT_EQ(0, ft_hash_string31(ft_nullptr));
    return (1);
}

FT_TEST(test_basic_hash_string31_empty_string)
{
    FT_ASSERT_EQ(0, ft_hash_string31(""));
    return (1);
}

FT_TEST(test_basic_hash_string31_single_character)
{
    FT_ASSERT_EQ(97, ft_hash_string31("a"));
    return (1);
}

FT_TEST(test_basic_hash_string31_matches_known_java_value)
{
    FT_ASSERT_EQ(-1595926131LL, ft_hash_string31("Minecraft"));
    return (1);
}

FT_TEST(test_basic_hash_string31_matches_known_ascii_phrase)
{
    FT_ASSERT_EQ(-1880044555LL, ft_hash_string31("Hello, world!"));
    return (1);
}

FT_TEST(test_basic_hash_string31_is_deterministic)
{
    int64_t first_hash;
    int64_t second_hash;

    first_hash = ft_hash_string31("dimension_overworld");
    second_hash = ft_hash_string31("dimension_overworld");
    FT_ASSERT_EQ(first_hash, second_hash);
    FT_ASSERT_EQ(-15916123LL, first_hash);
    return (1);
}

FT_TEST(test_basic_hash_string31_distinguishes_case)
{
    FT_ASSERT(ft_hash_string31("creeper") != ft_hash_string31("Creeper"));
    return (1);
}

FT_TEST(test_basic_hash_string31_preserves_whitespace_significance)
{
    FT_ASSERT(ft_hash_string31("nether") != ft_hash_string31("nether "));
    FT_ASSERT(ft_hash_string31("nether") != ft_hash_string31(" nether"));
    return (1);
}

FT_TEST(test_basic_hash_string31_uses_unsigned_byte_values)
{
    char string_with_high_bytes[4];

    string_with_high_bytes[0] = static_cast<char>(0xFF);
    string_with_high_bytes[1] = static_cast<char>(0x80);
    string_with_high_bytes[2] = static_cast<char>(0x7F);
    string_with_high_bytes[3] = '\0';
    FT_ASSERT_EQ(249150, ft_hash_string31(string_with_high_bytes));
    return (1);
}

FT_TEST(test_basic_hash_string31_ft_string_matches_c_string)
{
    ft_string string_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.assign("Minecraft", 9));
    FT_ASSERT_EQ(ft_hash_string31("Minecraft"), ft_hash_string31(string_value));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.destroy());
    return (1);
}

FT_TEST(test_basic_hash_string31_ft_string_empty_matches_c_string)
{
    ft_string string_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.initialize());
    FT_ASSERT_EQ(ft_hash_string31(""), ft_hash_string31(string_value));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.destroy());
    return (1);
}
