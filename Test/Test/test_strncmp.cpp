#include "../../Basic/basic.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_strncmp_prefix_equal, "ft_strncmp equal prefix")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(0, ft_strncmp("abcdef", "abcxyz", 3));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_strncmp_less, "ft_strncmp less")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT(ft_strncmp("abc", "abd", 3) < 0);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_strncmp_greater, "ft_strncmp greater")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT(ft_strncmp("abd", "abc", 3) > 0);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_strncmp_zero_length, "ft_strncmp zero length")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(0, ft_strncmp("abc", "xyz", 0));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_strncmp_zero_length_null_arguments, "ft_strncmp zero length allows null pointers")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(0, ft_strncmp(ft_nullptr, "abc", 0));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(0, ft_strncmp("abc", ft_nullptr, 0));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(0, ft_strncmp(ft_nullptr, ft_nullptr, 0));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_strncmp_shorter_first, "ft_strncmp shorter first")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT(ft_strncmp("ab", "abc", 3) < 0);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_strncmp_shorter_second, "ft_strncmp shorter second")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT(ft_strncmp("abc", "ab", 3) > 0);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_strncmp_null_arguments, "ft_strncmp null arguments return error")
{
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(-1, ft_strncmp(ft_nullptr, "abc", 3));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(-1, ft_strncmp("abc", ft_nullptr, 3));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_strncmp_high_bit_values, "ft_strncmp orders high-bit characters")
{
    char first[3];
    char second[3];

    first[0] = static_cast<char>(0x90);
    first[1] = 'a';
    first[2] = '\0';
    second[0] = static_cast<char>(0x10);
    second[1] = 'a';
    second[2] = '\0';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT(ft_strncmp(first, second, 2) > 0);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_strncmp_embedded_nulls_stop_comparison, "ft_strncmp stops comparing after embedded null")
{
    char first[5];
    char second[5];

    first[0] = 'a';
    first[1] = 'b';
    first[2] = '\0';
    first[3] = 'c';
    first[4] = '\0';
    second[0] = 'a';
    second[1] = 'b';
    second[2] = '\0';
    second[3] = 'd';
    second[4] = '\0';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(0, ft_strncmp(first, second, 4));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_strncmp_recovers_after_null_failure, "ft_strncmp clears errno after null pointer failure")
{
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(-1, ft_strncmp(ft_nullptr, "abc", 2));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT(ft_strncmp("abc", "abd", 3) < 0);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_strncmp_limit_blocks_mismatch_after_error, "ft_strncmp respects length after prior failure")
{
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(-1, ft_strncmp("abc", ft_nullptr, 2));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(0, ft_strncmp("prefix", "prelude", 3));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_strncmp_stops_at_null_within_limit, "ft_strncmp treats early terminator as end of comparison")
{
    char left[5];
    char right[5];

    left[0] = 'a';
    left[1] = 'b';
    left[2] = '\0';
    left[3] = 'x';
    left[4] = '\0';
    right[0] = 'a';
    right[1] = 'b';
    right[2] = 'c';
    right[3] = 'd';
    right[4] = '\0';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT(ft_strncmp(left, right, 4) < 0);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}
