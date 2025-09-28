#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

static void to_upper_iter(unsigned int index, char *character)
{
    (void)index;
    if (*character >= 'a' && *character <= 'z')
        *character -= 32;
    return ;
}

static unsigned int g_striteri_index_capture[8];
static unsigned int g_striteri_call_count = 0;

static void record_index_iter(unsigned int index, char *character)
{
    if (g_striteri_call_count < 8)
    {
        g_striteri_index_capture[g_striteri_call_count] = index;
        g_striteri_call_count++;
    }
    *character = static_cast<char>(*character + static_cast<char>(index + 1));
    return ;
}

FT_TEST(test_striteri_basic, "ft_striteri basic")
{
    char buffer[4];

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    buffer[3] = '\0';
    ft_striteri(buffer, to_upper_iter);
    FT_ASSERT_EQ(0, ft_strcmp("ABC", buffer));

    ft_striteri(ft_nullptr, to_upper_iter);
    ft_striteri(buffer, ft_nullptr);
    return (1);
}

FT_TEST(test_striteri_respects_terminator, "ft_striteri stops iterating at the null terminator")
{
    char buffer[6];

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = '\0';
    buffer[3] = 'x';
    buffer[4] = 'y';
    buffer[5] = '\0';
    g_striteri_call_count = 0;
    g_striteri_index_capture[0] = 99u;
    g_striteri_index_capture[1] = 99u;
    g_striteri_index_capture[2] = 99u;
    g_striteri_index_capture[3] = 99u;
    ft_striteri(buffer, record_index_iter);
    FT_ASSERT_EQ('b', buffer[0]);
    FT_ASSERT_EQ('d', buffer[1]);
    FT_ASSERT_EQ('\0', buffer[2]);
    FT_ASSERT_EQ('x', buffer[3]);
    FT_ASSERT_EQ('y', buffer[4]);
    FT_ASSERT_EQ('\0', buffer[5]);
    FT_ASSERT_EQ(2u, g_striteri_call_count);
    FT_ASSERT_EQ(0u, g_striteri_index_capture[0]);
    FT_ASSERT_EQ(1u, g_striteri_index_capture[1]);
    FT_ASSERT_EQ(99u, g_striteri_index_capture[2]);
    return (1);
}

FT_TEST(test_striteri_null_callback_no_modification, "ft_striteri ignores null callback and string")
{
    char buffer[5];
    char snapshot[5];

    buffer[0] = 'f';
    buffer[1] = 'o';
    buffer[2] = 'o';
    buffer[3] = '!';
    buffer[4] = '\0';
    snapshot[0] = buffer[0];
    snapshot[1] = buffer[1];
    snapshot[2] = buffer[2];
    snapshot[3] = buffer[3];
    snapshot[4] = buffer[4];
    g_striteri_call_count = 42u;
    ft_striteri(buffer, ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp(snapshot, buffer));
    ft_striteri(ft_nullptr, record_index_iter);
    FT_ASSERT_EQ(42u, g_striteri_call_count);
    return (1);
}

FT_TEST(test_striteri_valid_arguments_reset_errno, "ft_striteri clears ft_errno on valid input")
{
    char buffer[4];

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    buffer[3] = '\0';
    ft_errno = FT_EINVAL;
    ft_striteri(buffer, to_upper_iter);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_striteri_null_string_sets_errno, "ft_striteri sets FT_EINVAL when string is null")
{
    ft_errno = ER_SUCCESS;
    ft_striteri(ft_nullptr, to_upper_iter);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_striteri_null_function_sets_errno, "ft_striteri sets FT_EINVAL when callback is null")
{
    char buffer[2];

    buffer[0] = 'x';
    buffer[1] = '\0';
    ft_errno = ER_SUCCESS;
    ft_striteri(buffer, ft_nullptr);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}
