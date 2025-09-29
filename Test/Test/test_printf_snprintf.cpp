#include "../../Printf/printf.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include <cstdarg>
#include <cstdio>
#include <errno.h>

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

FT_TEST(test_pf_snprintf_null_string, "pf_snprintf returns error for null string")
{
    ft_errno = ER_SUCCESS;
    int result = pf_snprintf(static_cast<char *>(ft_nullptr), 8, "%s", "noop");
    FT_ASSERT_EQ(-1, result);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_pf_snprintf_null_format, "pf_snprintf returns error for null format")
{
    char buffer[8];
    typedef int (*t_pf_snprintf_plain)(char *, size_t, const char *, ...);
    t_pf_snprintf_plain call_pf_snprintf;

    buffer[0] = 'X';
    ft_errno = ER_SUCCESS;
    call_pf_snprintf = pf_snprintf;
    int result = call_pf_snprintf(buffer, sizeof(buffer), static_cast<const char *>(ft_nullptr));
    FT_ASSERT_EQ(-1, result);
    FT_ASSERT_EQ('\0', buffer[0]);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_pf_vsnprintf_null_format, "pf_vsnprintf returns error for null format")
{
    char buffer[8];

    buffer[0] = 'X';
    ft_errno = ER_SUCCESS;
    int result = pf_vsnprintf_wrapper(buffer, sizeof(buffer), static_cast<const char *>(ft_nullptr));
    FT_ASSERT_EQ(-1, result);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_pf_snprintf_tmpfile_failure, "pf_snprintf translates tmpfile failure")
{
    char buffer[8];

    buffer[0] = 'X';
    ft_errno = ER_SUCCESS;
    pf_set_tmpfile_function(pf_tmpfile_failure);
    int result = pf_snprintf(buffer, sizeof(buffer), "%s", "noop");
    pf_reset_tmpfile_function();
    FT_ASSERT_EQ(-1, result);
    FT_ASSERT_EQ('\0', buffer[0]);
    FT_ASSERT_EQ(FT_EALLOC, ft_errno);
    return (1);
}

FT_TEST(test_pf_snprintf_fflush_failure, "pf_snprintf translates fflush errno")
{
    char buffer[8];

    buffer[0] = 'X';
    ft_errno = ER_SUCCESS;
    errno = 0;
    pf_set_fflush_function(pf_fflush_failure);
    int result = pf_snprintf(buffer, sizeof(buffer), "%s", "noop");
    pf_reset_fflush_function();
    FT_ASSERT_EQ(-1, result);
    FT_ASSERT_EQ('\0', buffer[0]);
    FT_ASSERT_EQ(EIO + ERRNO_OFFSET, ft_errno);
    return (1);
}

FT_TEST(test_pf_snprintf_ftell_failure, "pf_snprintf translates ftell errno")
{
    char buffer[8];

    buffer[0] = 'X';
    ft_errno = ER_SUCCESS;
    errno = 0;
    pf_set_ftell_function(pf_ftell_failure);
    int result = pf_snprintf(buffer, sizeof(buffer), "%s", "noop");
    pf_reset_ftell_function();
    FT_ASSERT_EQ(-1, result);
    FT_ASSERT_EQ('\0', buffer[0]);
    FT_ASSERT_EQ(EIO + ERRNO_OFFSET, ft_errno);
    return (1);
}

FT_TEST(test_pf_snprintf_success_resets_errno, "pf_snprintf resets ft_errno on success")
{
    char buffer[16];

    ft_errno = FT_EINVAL;
    int result = pf_snprintf(buffer, sizeof(buffer), "%s", "ok");
    FT_ASSERT(result >= 0);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
