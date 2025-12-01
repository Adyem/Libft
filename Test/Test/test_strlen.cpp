#include "../../Libft/libft.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../CMA/CMA.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_strlen_nullptr, "ft_strlen nullptr")
{
    FT_ASSERT_EQ(0, ft_strlen(ft_nullptr));
    return (1);
}

FT_TEST(test_strlen_nullptr_sets_errno, "ft_strlen nullptr sets FT_ERR_INVALID_ARGUMENT")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_strlen(ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_strlen_stops_at_embedded_null, "ft_strlen stops counting at first null byte")
{
    char buffer[6];

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = '\0';
    buffer[3] = 'c';
    buffer[4] = 'd';
    buffer[5] = '\0';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(static_cast<size_t>(2), ft_strlen(buffer));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
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
    ft_errno = FT_ERR_OUT_OF_RANGE;
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
    ft_errno = FT_ERR_OUT_OF_RANGE;
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
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(static_cast<size_t>(2048), ft_strlen_size_t(buffer));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strlen_size_t_nullptr_sets_errno, "ft_strlen_size_t nullptr sets FT_ERR_INVALID_ARGUMENT")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(static_cast<size_t>(0), ft_strlen_size_t(ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
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
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    measured_length = ft_strlen_size_t(start_pointer);
    FT_ASSERT_EQ(static_cast<size_t>(5), measured_length);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strlen_size_t_recovers_after_nullptr, "ft_strlen_size_t clears errno after null input")
{
    char buffer[6];

    buffer[0] = 'v';
    buffer[1] = 'a';
    buffer[2] = 'l';
    buffer[3] = 'u';
    buffer[4] = 'e';
    buffer[5] = '\0';
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(static_cast<size_t>(0), ft_strlen_size_t(ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(static_cast<size_t>(5), ft_strlen_size_t(buffer));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strnlen_nullptr_sets_errno, "ft_strnlen nullptr sets FT_ERR_INVALID_ARGUMENT")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(static_cast<size_t>(0), ft_strnlen(ft_nullptr, 8));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_strnlen_truncates_to_maximum, "ft_strnlen caps measured length at maximum_length")
{
    static const char *source = "bounded-string";
    size_t measured_length;

    ft_errno = FT_ERR_OUT_OF_RANGE;
    measured_length = ft_strnlen(source, 6);
    FT_ASSERT_EQ(static_cast<size_t>(6), measured_length);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strnlen_returns_full_length_when_smaller_than_bound,
        "ft_strnlen returns string length when maximum_length exceeds it")
{
    static const char *source = "short";
    size_t measured_length;

    ft_errno = FT_ERR_OUT_OF_RANGE;
    measured_length = ft_strnlen(source, 32);
    FT_ASSERT_EQ(static_cast<size_t>(5), measured_length);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strnlen_zero_bound_resets_errno, "ft_strnlen zero bound clears errno")
{
    static const char *source = "unchanged";
    size_t measured_length;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    measured_length = ft_strnlen(source, 0);
    FT_ASSERT_EQ(static_cast<size_t>(0), measured_length);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_cma_strndup_nullptr_sets_errno, "cma_strndup nullptr sets FT_ERR_INVALID_ARGUMENT")
{
    char *duplicate;

    ft_errno = ER_SUCCESS;
    duplicate = cma_strndup(ft_nullptr, 4);
    FT_ASSERT_EQ(ft_nullptr, duplicate);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_cma_strndup_truncates_copy_to_requested_length, "cma_strndup copies at most maximum_length characters")
{
    char *duplicate;

    ft_errno = ER_SUCCESS;
    duplicate = cma_strndup("truncate-me", 3);
    FT_ASSERT(duplicate != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strncmp(duplicate, "tru", 4));
    FT_ASSERT_EQ('\0', duplicate[3]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(duplicate);
    return (1);
}

FT_TEST(test_cma_strndup_copies_full_string_when_shorter_than_bound,
        "cma_strndup duplicates entire string when it is shorter than maximum_length")
{
    char *duplicate;

    ft_errno = ER_SUCCESS;
    duplicate = cma_strndup("abc", 8);
    FT_ASSERT(duplicate != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strncmp(duplicate, "abc", 4));
    FT_ASSERT_EQ('\0', duplicate[3]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(duplicate);
    return (1);
}
FT_TEST(test_span_dup_copies_full_buffer_without_null, "ft_span_dup copies buffers without null terminators")
{
    char buffer[4];
    char *duplicate;

    buffer[0] = 'x';
    buffer[1] = 'y';
    buffer[2] = 'z';
    buffer[3] = 'w';
    ft_errno = FT_ERR_OUT_OF_RANGE;
    duplicate = ft_span_dup(buffer, 4);
    FT_ASSERT(duplicate != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strncmp(duplicate, "xyzw", 5));
    FT_ASSERT_EQ('\0', duplicate[4]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(duplicate);
    return (1);
}

FT_TEST(test_span_dup_handles_zero_length, "ft_span_dup allocates empty strings when length is zero")
{
    char buffer[1];
    char *duplicate;

    buffer[0] = 'q';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    duplicate = ft_span_dup(buffer, 0);
    FT_ASSERT(duplicate != ft_nullptr);
    FT_ASSERT_EQ('\0', duplicate[0]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(duplicate);
    return (1);
}

FT_TEST(test_span_dup_nullptr_with_length_sets_errno, "ft_span_dup nullptr with non-zero length sets FT_ERR_INVALID_ARGUMENT")
{
    char *duplicate;

    ft_errno = ER_SUCCESS;
    duplicate = ft_span_dup(ft_nullptr, 3);
    FT_ASSERT_EQ(ft_nullptr, duplicate);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_span_to_string_copies_span, "ft_span_to_string copies spans into ft_string")
{
    char buffer[5];
    ft_string span_string;

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    buffer[3] = 'd';
    buffer[4] = 'e';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    span_string = ft_span_to_string(buffer, 5);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(static_cast<size_t>(5), span_string.size());
    FT_ASSERT(span_string == "abcde");
    return (1);
}

FT_TEST(test_span_to_string_nullptr_with_length_sets_errno, "ft_span_to_string nullptr with non-zero length sets FT_ERR_INVALID_ARGUMENT")
{
    ft_string span_string;

    ft_errno = ER_SUCCESS;
    span_string = ft_span_to_string(ft_nullptr, 2);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, span_string.get_error());
    return (1);
}

FT_TEST(test_span_to_string_allows_empty_nullptr, "ft_span_to_string accepts nullptr when length is zero")
{
    ft_string span_string;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    span_string = ft_span_to_string(ft_nullptr, 0);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(static_cast<size_t>(0), span_string.size());
    FT_ASSERT(span_string.empty());
    return (1);
}
