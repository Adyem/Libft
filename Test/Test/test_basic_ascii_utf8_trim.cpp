#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_basic_isascii)
{
    FT_ASSERT_EQ(1, ft_isascii(0));
    FT_ASSERT_EQ(1, ft_isascii(0x7F));
    FT_ASSERT_EQ(0, ft_isascii(-1));
    FT_ASSERT_EQ(0, ft_isascii(0x80));
    return (1);
}

FT_TEST(test_basic_utf8_byte_classification)
{
    FT_ASSERT_EQ(1, ft_utf8_is_leading_byte('A'));
    FT_ASSERT_EQ(1, ft_utf8_is_leading_byte(0xC2));
    FT_ASSERT_EQ(1, ft_utf8_is_leading_byte(0xF4));
    FT_ASSERT_EQ(0, ft_utf8_is_leading_byte(0x80));
    FT_ASSERT_EQ(0, ft_utf8_is_leading_byte(0xC0));
    FT_ASSERT_EQ(1, ft_utf8_is_trailing_byte(0x80));
    FT_ASSERT_EQ(1, ft_utf8_is_trailing_byte(0xBF));
    FT_ASSERT_EQ(0, ft_utf8_is_trailing_byte('A'));
    FT_ASSERT_EQ(0, ft_utf8_is_trailing_byte(0xC2));
    return (1);
}

FT_TEST(test_basic_strtrim_left_in_place)
{
    char buffer[16];

    ft_strcpy_s(buffer, sizeof(buffer), "   hello");
    FT_ASSERT_EQ(buffer, ft_strtrim_left_in_place(buffer));
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "hello"));
    return (1);
}

FT_TEST(test_basic_strtrim_right_in_place)
{
    char buffer[16];

    ft_strcpy_s(buffer, sizeof(buffer), "hello   ");
    FT_ASSERT_EQ(buffer, ft_strtrim_right_in_place(buffer));
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "hello"));
    return (1);
}

FT_TEST(test_basic_strtrim_in_place)
{
    char buffer[16];

    ft_strcpy_s(buffer, sizeof(buffer), " \t hello \n");
    FT_ASSERT_EQ(buffer, ft_strtrim_in_place(buffer));
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "hello"));
    return (1);
}

FT_TEST(test_basic_strtrim_in_place_null_pointer)
{
    FT_ASSERT_EQ(ft_nullptr, ft_strtrim_left_in_place(ft_nullptr));
    FT_ASSERT_EQ(ft_nullptr, ft_strtrim_right_in_place(ft_nullptr));
    FT_ASSERT_EQ(ft_nullptr, ft_strtrim_in_place(ft_nullptr));
    return (1);
}
