#include "../test_internal.hpp"
#include "../../Printf/printf.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <errno.h>

#ifndef LIBFT_TEST_BUILD
#endif

extern void pf_set_tmpfile_function(FILE *(*function)(void));
extern void pf_set_fflush_function(int (*function)(FILE *));
extern void pf_set_ftell_function(long (*function)(FILE *));
extern void pf_reset_tmpfile_function(void);
extern void pf_reset_fflush_function(void);
extern void pf_reset_ftell_function(void);

static int pf_vsnprintf_wrapper(char *string, size_t size, const char *format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    int result = pf_vsnprintf(string, size, format, arguments);
    va_end(arguments);
    return (result);
}

static int std_vsnprintf_wrapper(char *string, size_t size, const char *format, ...)
{
    va_list arguments;

    va_start(arguments, format);
    int result = std::vsnprintf(string, size, format, arguments);
    va_end(arguments);
    return (result);
}

static FILE *pf_tmpfile_failure(void)
{
    return (static_cast<FILE *>(ft_nullptr));
}

static int pf_fflush_failure(FILE *stream)
{
    (void)stream;
    errno = EIO;
    return (-1);
}

static long pf_ftell_failure(FILE *stream)
{
    (void)stream;
    errno = EIO;
    return (-1);
}

static void build_repeated_string(char *buffer, size_t length, char character)
{
    size_t index;

    index = 0;
    while (index < length)
    {
        buffer[index] = character;
        index++;
    }
    buffer[length] = '\0';
    return ;
}

FT_TEST(test_pf_snprintf_null_string, "pf_snprintf returns error for null string")
{
    int result = pf_snprintf(static_cast<char *>(ft_nullptr), 8, "%s", "noop");
    FT_ASSERT_EQ(-1, result);
    return (1);
}

FT_TEST(test_pf_snprintf_null_string_zero_size, "pf_snprintf accepts null buffer when size is zero")
{
    int result = pf_snprintf(static_cast<char *>(ft_nullptr), 0, "%s", "noop");
    FT_ASSERT_EQ(4, result);
    return (1);
}

FT_TEST(test_pf_snprintf_null_format, "pf_snprintf returns error for null format")
{
    char buffer[8];
    typedef int (*t_pf_snprintf_plain)(char *, size_t, const char *, ...);
    t_pf_snprintf_plain call_pf_snprintf;

    buffer[0] = 'X';
    call_pf_snprintf = pf_snprintf;
    int result = call_pf_snprintf(buffer, sizeof(buffer), static_cast<const char *>(ft_nullptr));
    FT_ASSERT_EQ(-1, result);
    FT_ASSERT_EQ('\0', buffer[0]);
    return (1);
}

FT_TEST(test_pf_vsnprintf_null_format, "pf_vsnprintf returns error for null format")
{
    char buffer[8];

    buffer[0] = 'X';
    int result = pf_vsnprintf_wrapper(buffer, sizeof(buffer), static_cast<const char *>(ft_nullptr));
    FT_ASSERT_EQ(-1, result);
    return (1);
}

FT_TEST(test_pf_vsnprintf_null_string_zero_size, "pf_vsnprintf accepts null buffer when size is zero")
{
    int result = pf_vsnprintf_wrapper(static_cast<char *>(ft_nullptr), 0, "%s", "noop");
    FT_ASSERT_EQ(4, result);
    return (1);
}

FT_TEST(test_pf_snprintf_tmpfile_failure, "pf_snprintf translates tmpfile failure")
{
    char buffer[8];

    buffer[0] = 'X';
    pf_set_tmpfile_function(pf_tmpfile_failure);
    int result = pf_snprintf(buffer, sizeof(buffer), "%s", "noop");
    pf_reset_tmpfile_function();
    FT_ASSERT_EQ(-1, result);
    FT_ASSERT_EQ('\0', buffer[0]);
    return (1);
}

FT_TEST(test_pf_snprintf_fflush_failure, "pf_snprintf translates fflush errno")
{
    char buffer[8];

    buffer[0] = 'X';
    errno = 0;
    pf_set_fflush_function(pf_fflush_failure);
    int result = pf_snprintf(buffer, sizeof(buffer), "%s", "noop");
    pf_reset_fflush_function();
    FT_ASSERT_EQ(-1, result);
    FT_ASSERT_EQ('\0', buffer[0]);
    return (1);
}

FT_TEST(test_pf_snprintf_ftell_failure, "pf_snprintf translates ftell errno")
{
    char buffer[8];

    buffer[0] = 'X';
    errno = 0;
    pf_set_ftell_function(pf_ftell_failure);
    int result = pf_snprintf(buffer, sizeof(buffer), "%s", "noop");
    pf_reset_ftell_function();
    FT_ASSERT_EQ(-1, result);
    FT_ASSERT_EQ('\0', buffer[0]);
    return (1);
}

{
    char buffer[16];

    int result = pf_snprintf(buffer, sizeof(buffer), "%s", "ok");
    FT_ASSERT(result >= 0);
    return (1);
}

FT_TEST(test_pf_snprintf_matches_standard_exact_fit, "pf_snprintf matches std::snprintf for exact fits")
{
    char pf_buffer[32];
    char standard_buffer[32];

    std::memset(pf_buffer, 0, sizeof(pf_buffer));
    std::memset(standard_buffer, 0, sizeof(standard_buffer));

    int pf_result = pf_snprintf(pf_buffer, sizeof(pf_buffer), "%s %d", "value", 42);
    int standard_result = std::snprintf(standard_buffer, sizeof(standard_buffer), "%s %d", "value", 42);
    FT_ASSERT_EQ(standard_result, pf_result);
    FT_ASSERT_EQ(0, std::strcmp(standard_buffer, pf_buffer));
    return (1);
}

FT_TEST(test_pf_snprintf_matches_standard_truncation, "pf_snprintf matches std::snprintf on truncation")
{
    char pf_buffer[6];
    char standard_buffer[6];
    char long_input[16];
    size_t long_length;

    std::memset(pf_buffer, 0, sizeof(pf_buffer));
    std::memset(standard_buffer, 0, sizeof(standard_buffer));
    long_length = 8;
    build_repeated_string(long_input, long_length, 't');

    int pf_result = pf_snprintf(pf_buffer, sizeof(pf_buffer), "%s", long_input);
    int standard_result = std::snprintf(standard_buffer, sizeof(standard_buffer), "%s", long_input);
    FT_ASSERT_EQ(standard_result, pf_result);
    FT_ASSERT_EQ(0, std::strcmp(standard_buffer, pf_buffer));
    return (1);
}

FT_TEST(test_pf_snprintf_matches_standard_zero_size, "pf_snprintf matches std::snprintf when size is zero")
{
    char pf_buffer[4];
    char standard_buffer[4];

    pf_buffer[0] = 'P';
    standard_buffer[0] = 'S';

    int pf_result = pf_snprintf(pf_buffer, 0, "%s", "noop");
    int standard_result = std::snprintf(standard_buffer, 0, "%s", "noop");
    FT_ASSERT_EQ(standard_result, pf_result);
    FT_ASSERT_EQ('P', pf_buffer[0]);
    FT_ASSERT_EQ('S', standard_buffer[0]);
    return (1);
}

FT_TEST(test_pf_vsnprintf_matches_standard_exact_fit, "pf_vsnprintf matches std::vsnprintf for exact fits")
{
    char pf_buffer[48];
    char standard_buffer[48];

    std::memset(pf_buffer, 0, sizeof(pf_buffer));
    std::memset(standard_buffer, 0, sizeof(standard_buffer));

    int pf_result = pf_vsnprintf_wrapper(pf_buffer, sizeof(pf_buffer), "%s %d %c", "value", 42, 'Z');
    int standard_result = std_vsnprintf_wrapper(standard_buffer, sizeof(standard_buffer), "%s %d %c", "value", 42, 'Z');
    FT_ASSERT_EQ(standard_result, pf_result);
    FT_ASSERT_EQ(0, std::strcmp(standard_buffer, pf_buffer));
    return (1);
}

FT_TEST(test_pf_vsnprintf_matches_standard_truncation, "pf_vsnprintf matches std::vsnprintf on truncation")
{
    char pf_buffer[8];
    char standard_buffer[8];
    char long_input[32];
    size_t long_length;

    std::memset(pf_buffer, 0, sizeof(pf_buffer));
    std::memset(standard_buffer, 0, sizeof(standard_buffer));
    long_length = 16;
    build_repeated_string(long_input, long_length, 'v');

    int pf_result = pf_vsnprintf_wrapper(pf_buffer, sizeof(pf_buffer), "%s", long_input);
    int standard_result = std_vsnprintf_wrapper(standard_buffer, sizeof(standard_buffer), "%s", long_input);
    FT_ASSERT_EQ(standard_result, pf_result);
    FT_ASSERT_EQ(0, std::strcmp(standard_buffer, pf_buffer));
    return (1);
}
