#include "../../Libft/libft.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_strlen_nullptr, "ft_strlen nullptr")
{
    FT_ASSERT_EQ(0, ft_strlen(ft_nullptr));
    return (1);
}

FT_TEST(test_strlen_nullptr_sets_errno, "ft_strlen nullptr sets FT_EINVAL")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_strlen(ft_nullptr));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_strlen_simple, "ft_strlen basic")
{
    FT_ASSERT_EQ(4, ft_strlen("test"));
    return (1);
}

FT_TEST(test_strlen_long, "ft_strlen long string")
{
    static char buffer[1025];
    int index;

    index = 0;
    while (index < 1024)
    {
        buffer[index] = 'a';
        index++;
    }
    buffer[1024] = '\0';
    FT_ASSERT_EQ(1024, ft_strlen(buffer));
    return (1);
}

FT_TEST(test_strlen_empty, "ft_strlen empty string")
{
    FT_ASSERT_EQ(0, ft_strlen(""));
    return (1);
}

FT_TEST(test_strlen_embedded_null, "ft_strlen embedded null")
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

FT_TEST(test_strlen_resets_errno_on_success, "ft_strlen clears ft_errno before measuring")
{
    ft_errno = FT_ERANGE;
    FT_ASSERT_EQ(3, ft_strlen("abc"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strlen_counts_non_ascii_bytes, "ft_strlen counts bytes beyond ascii")
{
    char string[3];

    string[0] = static_cast<char>(0xFF);
    string[1] = static_cast<char>(0x80);
    string[2] = '\0';
    ft_errno = FT_ERANGE;
    FT_ASSERT_EQ(2, ft_strlen(string));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strlen_size_t_large_buffer, "ft_strlen_size_t handles wide buffers")
{
    static char buffer[2049];
    size_t index;

    index = 0;
    while (index < 2048)
    {
        buffer[index] = 'x';
        index++;
    }
    buffer[2048] = '\0';
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(static_cast<size_t>(2048), ft_strlen_size_t(buffer));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strlen_size_t_nullptr_sets_errno, "ft_strlen_size_t nullptr sets FT_EINVAL")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(static_cast<size_t>(0), ft_strlen_size_t(ft_nullptr));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_strlen_size_t_handles_unaligned_pointers,
        "ft_strlen_size_t measures strings that begin at unaligned addresses")
{
    char buffer[7];
    size_t index;
    char *start_pointer;
    size_t measured_length;

    index = 0;
    while (index < 6)
    {
        buffer[index] = 'a';
        index++;
    }
    buffer[6] = '\0';
    start_pointer = buffer + 1;
    ft_errno = FT_EINVAL;
    measured_length = ft_strlen_size_t(start_pointer);
    FT_ASSERT_EQ(static_cast<size_t>(5), measured_length);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
