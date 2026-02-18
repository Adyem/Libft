#include "../test_internal.hpp"
#include "../../Basic/basic.hpp"
#include "../../CMA/CMA.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_strlen_nullptr, "ft_strlen returns zero for null input")
{
    FT_ASSERT_EQ(0, ft_strlen(ft_nullptr));
    return (1);
}

FT_TEST(test_strlen_simple, "ft_strlen measures string length")
{
    FT_ASSERT_EQ(4, ft_strlen("test"));
    return (1);
}

FT_TEST(test_strlen_embedded_null, "ft_strlen stops at first null byte")
{
    char string[6];

    string[0] = 'a';
    string[1] = 'b';
    string[2] = '\0';
    string[3] = 'c';
    string[4] = 'd';
    string[5] = '\0';
    FT_ASSERT_EQ(2, ft_strlen(string));
    return (1);
}

FT_TEST(test_strlen_size_t_large_buffer, "ft_strlen_size_t handles long strings")
{
    static char buffer[2049];
    ft_size_t index;

    index = 0;
    while (index < 2048)
    {
        buffer[index] = 'x';
        index++;
    }
    buffer[2048] = '\0';
    FT_ASSERT_EQ(2048, ft_strlen_size_t(buffer));
    return (1);
}

FT_TEST(test_strlen_size_t_nullptr, "ft_strlen_size_t returns zero for null input")
{
    FT_ASSERT_EQ(0, ft_strlen_size_t(ft_nullptr));
    return (1);
}

FT_TEST(test_strnlen_truncates_to_bound, "ft_strnlen caps measured length")
{
    FT_ASSERT_EQ(6, ft_strnlen("bounded-string", 6));
    return (1);
}

FT_TEST(test_strnlen_returns_full_length, "ft_strnlen returns full length when bound is larger")
{
    FT_ASSERT_EQ(5, ft_strnlen("short", 32));
    return (1);
}

FT_TEST(test_span_dup_copies_without_embedded_null_requirement, "ft_span_dup copies raw bytes and appends terminator")
{
    char buffer[4];
    char *duplicate;

    buffer[0] = 'x';
    buffer[1] = 'y';
    buffer[2] = 'z';
    buffer[3] = 'w';
    duplicate = ft_span_dup(buffer, 4);
    FT_ASSERT(duplicate != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strncmp(duplicate, "xyzw", 5));
    FT_ASSERT_EQ('\0', duplicate[4]);
    cma_free(duplicate);
    return (1);
}
