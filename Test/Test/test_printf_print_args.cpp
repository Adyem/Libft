#include "../../Printf/printf_internal.hpp"
#include "../../Basic/basic.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cstddef>
#include <unistd.h>
#include <cstdio>
#include <climits>
#include <limits>

static int create_pipe(int pipe_fds[2])
{
    if (pipe(pipe_fds) != 0)
        return (0);
    return (1);
}

static int close_pipe_end(int file_descriptor)
{
    if (close(file_descriptor) != 0)
        return (0);
    return (1);
}

static int read_pipe_into_buffer(int read_fd, char *buffer, size_t buffer_size, ssize_t *bytes_read)
{
    if (buffer == ft_nullptr || bytes_read == ft_nullptr)
        return (0);
    *bytes_read = read(read_fd, buffer, buffer_size);
    if (*bytes_read < 0)
        return (0);
    return (1);
}

FT_TEST(test_ft_putchar_fd_writes_character, "ft_putchar_fd writes a single byte and updates count")
{
    int pipe_fds[2];
    size_t write_count;
    ssize_t bytes_read;
    char buffer[2];

    FT_ASSERT(create_pipe(pipe_fds));
    write_count = 0;
    ft_putchar_fd('Z', pipe_fds[1], &write_count);
    FT_ASSERT_EQ(static_cast<size_t>(1), write_count);
    FT_ASSERT(close_pipe_end(pipe_fds[1]));
    FT_ASSERT(read_pipe_into_buffer(pipe_fds[0], buffer, 1, &bytes_read));
    FT_ASSERT_EQ(static_cast<ssize_t>(1), bytes_read);
    buffer[static_cast<size_t>(bytes_read)] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "Z"));
    FT_ASSERT(close_pipe_end(pipe_fds[0]));
    return (1);
}

FT_TEST(test_ft_putchar_fd_ignores_null_count, "ft_putchar_fd skips writing when count pointer is null")
{
    int pipe_fds[2];
    ssize_t bytes_read;
    char buffer[2];

    FT_ASSERT(create_pipe(pipe_fds));
    ft_putchar_fd('A', pipe_fds[1], ft_nullptr);
    FT_ASSERT(close_pipe_end(pipe_fds[1]));
    FT_ASSERT(read_pipe_into_buffer(pipe_fds[0], buffer, 1, &bytes_read));
    FT_ASSERT_EQ(static_cast<ssize_t>(0), bytes_read);
    FT_ASSERT(close_pipe_end(pipe_fds[0]));
    return (1);
}

FT_TEST(test_ft_putnbr_fd_writes_negative_number, "ft_putnbr_fd prints negative values with sign and updates count")
{
    int pipe_fds[2];
    size_t write_count;
    ssize_t bytes_read;
    char buffer[32];

    FT_ASSERT(create_pipe(pipe_fds));
    write_count = 0;
    ft_putnbr_fd(-12345, pipe_fds[1], &write_count);
    FT_ASSERT_EQ(static_cast<size_t>(6), write_count);
    FT_ASSERT(close_pipe_end(pipe_fds[1]));
    FT_ASSERT(read_pipe_into_buffer(pipe_fds[0], buffer, sizeof(buffer) - 1, &bytes_read));
    FT_ASSERT(bytes_read >= 0);
    buffer[static_cast<size_t>(bytes_read)] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "-12345"));
    FT_ASSERT(close_pipe_end(pipe_fds[0]));
    return (1);
}

FT_TEST(test_ft_putnbr_fd_handles_long_min, "ft_putnbr_fd prints LONG_MIN without overflow")
{
    int pipe_fds[2];
    size_t write_count;
    ssize_t bytes_read;
    char buffer[64];
    char expected[64];
    int expected_length;
    long value;

    FT_ASSERT(create_pipe(pipe_fds));
    value = LONG_MIN;
    expected_length = std::snprintf(expected, sizeof(expected), "%ld", value);
    FT_ASSERT(expected_length > 0);
    write_count = 0;
    ft_putnbr_fd(value, pipe_fds[1], &write_count);
    FT_ASSERT_EQ(static_cast<size_t>(expected_length), write_count);
    FT_ASSERT(close_pipe_end(pipe_fds[1]));
    FT_ASSERT(read_pipe_into_buffer(pipe_fds[0], buffer, sizeof(buffer) - 1, &bytes_read));
    FT_ASSERT(bytes_read >= 0);
    buffer[static_cast<size_t>(bytes_read)] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(buffer, expected));
    FT_ASSERT(close_pipe_end(pipe_fds[0]));
    return (1);
}

FT_TEST(test_ft_puthex_fd_respects_uppercase_flag, "ft_puthex_fd selects uppercase digits when requested")
{
    int pipe_fds[2];
    size_t write_count;
    ssize_t bytes_read;
    char buffer[32];

    FT_ASSERT(create_pipe(pipe_fds));
    write_count = 0;
    ft_puthex_fd(0x1FAB, pipe_fds[1], true, &write_count);
    FT_ASSERT_EQ(static_cast<size_t>(4), write_count);
    FT_ASSERT(close_pipe_end(pipe_fds[1]));
    FT_ASSERT(read_pipe_into_buffer(pipe_fds[0], buffer, sizeof(buffer) - 1, &bytes_read));
    FT_ASSERT(bytes_read >= 0);
    buffer[static_cast<size_t>(bytes_read)] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "1FAB"));
    FT_ASSERT(close_pipe_end(pipe_fds[0]));
    return (1);
}

FT_TEST(test_ft_puthex_fd_lowercase_output, "ft_puthex_fd emits lowercase digits when flag disabled")
{
    int pipe_fds[2];
    size_t write_count;
    ssize_t bytes_read;
    char buffer[32];

    FT_ASSERT(create_pipe(pipe_fds));
    write_count = 0;
    ft_puthex_fd(0xBEEF, pipe_fds[1], false, &write_count);
    FT_ASSERT_EQ(static_cast<size_t>(4), write_count);
    FT_ASSERT(close_pipe_end(pipe_fds[1]));
    FT_ASSERT(read_pipe_into_buffer(pipe_fds[0], buffer, sizeof(buffer) - 1, &bytes_read));
    FT_ASSERT(bytes_read >= 0);
    buffer[static_cast<size_t>(bytes_read)] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "beef"));
    FT_ASSERT(close_pipe_end(pipe_fds[0]));
    return (1);
}

FT_TEST(test_ft_puthex_fd_zero_value, "ft_puthex_fd prints zero without extra digits")
{
    int pipe_fds[2];
    size_t write_count;
    ssize_t bytes_read;
    char buffer[8];

    FT_ASSERT(create_pipe(pipe_fds));
    write_count = 0;
    ft_puthex_fd(0, pipe_fds[1], false, &write_count);
    FT_ASSERT_EQ(static_cast<size_t>(1), write_count);
    FT_ASSERT(close_pipe_end(pipe_fds[1]));
    FT_ASSERT(read_pipe_into_buffer(pipe_fds[0], buffer, sizeof(buffer) - 1, &bytes_read));
    FT_ASSERT(bytes_read >= 0);
    buffer[static_cast<size_t>(bytes_read)] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "0"));
    FT_ASSERT(close_pipe_end(pipe_fds[0]));
    return (1);
}

FT_TEST(test_ft_putfloat_fd_matches_snprintf, "ft_putfloat_fd mirrors standard formatting at given precision")
{
    int pipe_fds[2];
    size_t write_count;
    ssize_t bytes_read;
    char buffer[64];
    char expected[64];
    int expected_length;

    FT_ASSERT(create_pipe(pipe_fds));
    expected_length = std::snprintf(expected, sizeof(expected), "%.3f", 3.14159);
    FT_ASSERT(expected_length > 0);
    write_count = 0;
    ft_putfloat_fd(3.14159, pipe_fds[1], &write_count, 3);
    FT_ASSERT_EQ(static_cast<size_t>(expected_length), write_count);
    FT_ASSERT(close_pipe_end(pipe_fds[1]));
    FT_ASSERT(read_pipe_into_buffer(pipe_fds[0], buffer, sizeof(buffer) - 1, &bytes_read));
    FT_ASSERT(bytes_read >= 0);
    buffer[static_cast<size_t>(bytes_read)] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(buffer, expected));
    FT_ASSERT(close_pipe_end(pipe_fds[0]));
    return (1);
}

FT_TEST(test_ft_putgeneral_fd_uppercase, "ft_putgeneral_fd honors uppercase flag and precision")
{
    int pipe_fds[2];
    size_t write_count;
    ssize_t bytes_read;
    char buffer[64];
    char expected[64];
    int expected_length;
    double value;

    FT_ASSERT(create_pipe(pipe_fds));
    value = 12345.6789;
    expected_length = std::snprintf(expected, sizeof(expected), "%.*G", 4, value);
    FT_ASSERT(expected_length > 0);
    write_count = 0;
    ft_putgeneral_fd(value, true, pipe_fds[1], &write_count, 4);
    FT_ASSERT_EQ(static_cast<size_t>(expected_length), write_count);
    FT_ASSERT(close_pipe_end(pipe_fds[1]));
    FT_ASSERT(read_pipe_into_buffer(pipe_fds[0], buffer, sizeof(buffer) - 1, &bytes_read));
    FT_ASSERT(bytes_read >= 0);
    buffer[static_cast<size_t>(bytes_read)] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(buffer, expected));
    FT_ASSERT(close_pipe_end(pipe_fds[0]));
    return (1);
}

FT_TEST(test_ft_putstr_fd_writes_string, "ft_putstr_fd streams the provided string and updates count")
{
    int pipe_fds[2];
    size_t write_count;
    ssize_t bytes_read;
    char buffer[32];

    FT_ASSERT(create_pipe(pipe_fds));
    write_count = 0;
    ft_putstr_fd("hello", pipe_fds[1], &write_count);
    FT_ASSERT_EQ(static_cast<size_t>(5), write_count);
    FT_ASSERT(close_pipe_end(pipe_fds[1]));
    FT_ASSERT(read_pipe_into_buffer(pipe_fds[0], buffer, sizeof(buffer) - 1, &bytes_read));
    FT_ASSERT(bytes_read >= 0);
    buffer[static_cast<size_t>(bytes_read)] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "hello"));
    FT_ASSERT(close_pipe_end(pipe_fds[0]));
    return (1);
}

FT_TEST(test_ft_putstr_fd_null_pointer_writes_literal, "ft_putstr_fd prints (null) when given a null pointer")
{
    int pipe_fds[2];
    size_t write_count;
    ssize_t bytes_read;
    char buffer[32];

    FT_ASSERT(create_pipe(pipe_fds));
    write_count = 0;
    ft_putstr_fd(ft_nullptr, pipe_fds[1], &write_count);
    FT_ASSERT_EQ(static_cast<size_t>(6), write_count);
    FT_ASSERT(close_pipe_end(pipe_fds[1]));
    FT_ASSERT(read_pipe_into_buffer(pipe_fds[0], buffer, sizeof(buffer) - 1, &bytes_read));
    FT_ASSERT(bytes_read >= 0);
    buffer[static_cast<size_t>(bytes_read)] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "(null)"));
    FT_ASSERT(close_pipe_end(pipe_fds[0]));
    return (1);
}

FT_TEST(test_ft_putunsigned_fd_prints_decimal, "ft_putunsigned_fd renders the full decimal representation")
{
    int pipe_fds[2];
    size_t write_count;
    ssize_t bytes_read;
    char buffer[64];
    char expected[64];
    int expected_length;

    FT_ASSERT(create_pipe(pipe_fds));
    expected_length = std::snprintf(expected, sizeof(expected), "%ju", static_cast<uintmax_t>(1234567890));
    FT_ASSERT(expected_length > 0);
    write_count = 0;
    ft_putunsigned_fd(1234567890, pipe_fds[1], &write_count);
    FT_ASSERT_EQ(static_cast<size_t>(expected_length), write_count);
    FT_ASSERT(close_pipe_end(pipe_fds[1]));
    FT_ASSERT(read_pipe_into_buffer(pipe_fds[0], buffer, sizeof(buffer) - 1, &bytes_read));
    FT_ASSERT(bytes_read >= 0);
    buffer[static_cast<size_t>(bytes_read)] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(buffer, expected));
    FT_ASSERT(close_pipe_end(pipe_fds[0]));
    return (1);
}

FT_TEST(test_ft_putunsigned_fd_uintmax_max, "ft_putunsigned_fd prints UINTMAX_MAX accurately")
{
    int pipe_fds[2];
    size_t write_count;
    ssize_t bytes_read;
    char buffer[128];
    char expected[128];
    int expected_length;
    uintmax_t value;

    FT_ASSERT(create_pipe(pipe_fds));
    value = std::numeric_limits<uintmax_t>::max();
    expected_length = std::snprintf(expected, sizeof(expected), "%ju", static_cast<uintmax_t>(value));
    FT_ASSERT(expected_length > 0);
    write_count = 0;
    ft_putunsigned_fd(value, pipe_fds[1], &write_count);
    FT_ASSERT_EQ(static_cast<size_t>(expected_length), write_count);
    FT_ASSERT(close_pipe_end(pipe_fds[1]));
    FT_ASSERT(read_pipe_into_buffer(pipe_fds[0], buffer, sizeof(buffer) - 1, &bytes_read));
    FT_ASSERT(bytes_read >= 0);
    buffer[static_cast<size_t>(bytes_read)] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(buffer, expected));
    FT_ASSERT(close_pipe_end(pipe_fds[0]));
    return (1);
}

FT_TEST(test_ft_putunsigned_fd_zero_value, "ft_putunsigned_fd prints zero without extra digits")
{
    int pipe_fds[2];
    size_t write_count;
    ssize_t bytes_read;
    char buffer[8];

    FT_ASSERT(create_pipe(pipe_fds));
    write_count = 0;
    ft_putunsigned_fd(0, pipe_fds[1], &write_count);
    FT_ASSERT_EQ(static_cast<size_t>(1), write_count);
    FT_ASSERT(close_pipe_end(pipe_fds[1]));
    FT_ASSERT(read_pipe_into_buffer(pipe_fds[0], buffer, sizeof(buffer) - 1, &bytes_read));
    FT_ASSERT(bytes_read >= 0);
    buffer[static_cast<size_t>(bytes_read)] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "0"));
    FT_ASSERT(close_pipe_end(pipe_fds[0]));
    return (1);
}

FT_TEST(test_ft_putoctal_fd_outputs_octal_digits, "ft_putoctal_fd emits octal representation")
{
    int pipe_fds[2];
    size_t write_count;
    ssize_t bytes_read;
    char buffer[32];
    char expected[32];
    int expected_length;

    FT_ASSERT(create_pipe(pipe_fds));
    expected_length = std::snprintf(expected, sizeof(expected), "%llo", static_cast<unsigned long long>(0754321));
    FT_ASSERT(expected_length > 0);
    write_count = 0;
    ft_putoctal_fd(0754321, pipe_fds[1], &write_count);
    FT_ASSERT_EQ(static_cast<size_t>(expected_length), write_count);
    FT_ASSERT(close_pipe_end(pipe_fds[1]));
    FT_ASSERT(read_pipe_into_buffer(pipe_fds[0], buffer, sizeof(buffer) - 1, &bytes_read));
    FT_ASSERT(bytes_read >= 0);
    buffer[static_cast<size_t>(bytes_read)] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(buffer, expected));
    FT_ASSERT(close_pipe_end(pipe_fds[0]));
    return (1);
}

FT_TEST(test_ft_putptr_fd_formats_address, "ft_putptr_fd prefixes addresses with 0x and hexadecimal digits")
{
    int pipe_fds[2];
    size_t write_count;
    ssize_t bytes_read;
    char buffer[64];
    char expected[64];
    int expected_length;
    int sample_value;
    uintptr_t address_value;

    FT_ASSERT(create_pipe(pipe_fds));
    sample_value = 42;
    address_value = reinterpret_cast<uintptr_t>(&sample_value);
    expected_length = std::snprintf(expected, sizeof(expected), "0x%llx", static_cast<unsigned long long>(address_value));
    FT_ASSERT(expected_length > 0);
    write_count = 0;
    ft_putptr_fd(&sample_value, pipe_fds[1], &write_count);
    FT_ASSERT_EQ(static_cast<size_t>(expected_length), write_count);
    FT_ASSERT(close_pipe_end(pipe_fds[1]));
    FT_ASSERT(read_pipe_into_buffer(pipe_fds[0], buffer, sizeof(buffer) - 1, &bytes_read));
    FT_ASSERT(bytes_read >= 0);
    buffer[static_cast<size_t>(bytes_read)] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(buffer, expected));
    FT_ASSERT(close_pipe_end(pipe_fds[0]));
    return (1);
}

FT_TEST(test_ft_putptr_fd_null_pointer, "ft_putptr_fd prints zero address for null pointers")
{
    int pipe_fds[2];
    size_t write_count;
    ssize_t bytes_read;
    char buffer[16];

    FT_ASSERT(create_pipe(pipe_fds));
    write_count = 0;
    ft_putptr_fd(ft_nullptr, pipe_fds[1], &write_count);
    FT_ASSERT_EQ(static_cast<size_t>(3), write_count);
    FT_ASSERT(close_pipe_end(pipe_fds[1]));
    FT_ASSERT(read_pipe_into_buffer(pipe_fds[0], buffer, sizeof(buffer) - 1, &bytes_read));
    FT_ASSERT(bytes_read >= 0);
    buffer[static_cast<size_t>(bytes_read)] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "0x0"));
    FT_ASSERT(close_pipe_end(pipe_fds[0]));
    return (1);
}

FT_TEST(test_ft_putscientific_fd_lowercase, "ft_putscientific_fd prints lowercase exponential notation by default")
{
    int pipe_fds[2];
    size_t write_count;
    ssize_t bytes_read;
    char buffer[64];
    char expected[64];
    int expected_length;
    double value;

    FT_ASSERT(create_pipe(pipe_fds));
    value = 9876.54321;
    expected_length = std::snprintf(expected, sizeof(expected), "%.*e", 5, value);
    FT_ASSERT(expected_length > 0);
    write_count = 0;
    ft_putscientific_fd(value, false, pipe_fds[1], &write_count, 5);
    FT_ASSERT_EQ(static_cast<size_t>(expected_length), write_count);
    FT_ASSERT(close_pipe_end(pipe_fds[1]));
    FT_ASSERT(read_pipe_into_buffer(pipe_fds[0], buffer, sizeof(buffer) - 1, &bytes_read));
    FT_ASSERT(bytes_read >= 0);
    buffer[static_cast<size_t>(bytes_read)] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(buffer, expected));
    FT_ASSERT(close_pipe_end(pipe_fds[0]));
    return (1);
}

FT_TEST(test_ft_putscientific_fd_uppercase, "ft_putscientific_fd honors uppercase flag")
{
    int pipe_fds[2];
    size_t write_count;
    ssize_t bytes_read;
    char buffer[64];
    char expected[64];
    int expected_length;
    double value;

    FT_ASSERT(create_pipe(pipe_fds));
    value = 0.0012345;
    expected_length = std::snprintf(expected, sizeof(expected), "%.*E", 3, value);
    FT_ASSERT(expected_length > 0);
    write_count = 0;
    ft_putscientific_fd(value, true, pipe_fds[1], &write_count, 3);
    FT_ASSERT_EQ(static_cast<size_t>(expected_length), write_count);
    FT_ASSERT(close_pipe_end(pipe_fds[1]));
    FT_ASSERT(read_pipe_into_buffer(pipe_fds[0], buffer, sizeof(buffer) - 1, &bytes_read));
    FT_ASSERT(bytes_read >= 0);
    buffer[static_cast<size_t>(bytes_read)] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(buffer, expected));
    FT_ASSERT(close_pipe_end(pipe_fds[0]));
    return (1);
}

FT_TEST(test_ft_putstr_fd_skips_when_count_in_error, "ft_putstr_fd does not write when count already signals error")
{
    int pipe_fds[2];
    size_t write_count;
    ssize_t bytes_read;
    char buffer[8];

    FT_ASSERT(create_pipe(pipe_fds));
    write_count = SIZE_MAX;
    ft_putstr_fd("ignored", pipe_fds[1], &write_count);
    FT_ASSERT_EQ(SIZE_MAX, write_count);
    FT_ASSERT(close_pipe_end(pipe_fds[1]));
    FT_ASSERT(read_pipe_into_buffer(pipe_fds[0], buffer, sizeof(buffer) - 1, &bytes_read));
    FT_ASSERT_EQ(static_cast<ssize_t>(0), bytes_read);
    FT_ASSERT(close_pipe_end(pipe_fds[0]));
    return (1);
}

FT_TEST(test_ft_putnbr_fd_marks_error_on_failed_write, "ft_putnbr_fd marks count as error when su_write fails")
{
    size_t write_count;

    write_count = 0;
    ft_errno = FT_ERR_SUCCESSS;
    ft_putnbr_fd(42, -1, &write_count);
    FT_ASSERT_EQ(SIZE_MAX, write_count);
    FT_ASSERT(ft_errno != FT_ERR_SUCCESSS);
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_ft_strlen_printf_handles_null, "ft_strlen_printf treats null pointers as length six")
{
    FT_ASSERT_EQ(static_cast<size_t>(6), ft_strlen_printf(ft_nullptr));
    FT_ASSERT_EQ(static_cast<size_t>(4), ft_strlen_printf("test"));
    return (1);
}
