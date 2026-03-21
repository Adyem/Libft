#include "../test_internal.hpp"
#include "../../Basic/basic.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static int assert_buffer_zeroed(const char *buffer, size_t buffer_size)
{
    size_t index;

    index = 0;
    while (index < buffer_size)
    {
        FT_ASSERT(buffer[index] == '\0');
        index++;
    }
    return (1);
}

FT_TEST(test_strncpy_s_copies_within_bounds)
{
    char destination[16];
    const char *source;

    source = "hello";
    ft_memset(destination, 'X', sizeof(destination));
    FT_ASSERT_EQ(0, ft_strncpy_s(destination, sizeof(destination), source, 10));
    FT_ASSERT_EQ(0, ft_strcmp(source, destination));
    return (1);
}

FT_TEST(test_strncpy_s_detects_truncation)
{
    char destination[8];
    const char *source;

    source = "toolong";
    ft_memset(destination, 'Y', sizeof(destination));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT,
            ft_strncpy_s(destination, sizeof(destination), source, 4));
    if (!assert_buffer_zeroed(destination, sizeof(destination)))
        return (0);
    return (1);
}

FT_TEST(test_strncat_s_appends_without_truncation)
{
    char buffer[16];

    ft_memset(buffer, 0, sizeof(buffer));
    FT_ASSERT_EQ(0, ft_strcpy_s(buffer, sizeof(buffer), "foo"));
    FT_ASSERT_EQ(0, ft_strncat_s(buffer, sizeof(buffer), "bar", 8));
    FT_ASSERT_EQ(0, ft_strcmp("foobar", buffer));
    return (1);
}

FT_TEST(test_strncat_s_detects_capacity_overflow)
{
    char buffer[8];

    FT_ASSERT_EQ(0, ft_strcpy_s(buffer, sizeof(buffer), "data"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT,
            ft_strncat_s(buffer, sizeof(buffer), "more", 4));
    if (!assert_buffer_zeroed(buffer, sizeof(buffer)))
        return (0);
    return (1);
}

FT_TEST(test_strncat_s_respects_append_length_limit)
{
    char buffer[16];

    FT_ASSERT_EQ(0, ft_strcpy_s(buffer, sizeof(buffer), "base"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT,
            ft_strncat_s(buffer, sizeof(buffer), "suffix", 2));
    if (!assert_buffer_zeroed(buffer, sizeof(buffer)))
        return (0);
    return (1);
}
