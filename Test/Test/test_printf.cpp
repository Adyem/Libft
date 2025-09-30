#include "../../Printf/printf.hpp"
#include "../../Printf/printf_test_hooks.hpp"
#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <climits>
#include <fcntl.h>
#include <unistd.h>

static size_t pf_test_count_overflow_hook(void)
{
    return (static_cast<size_t>(INT_MAX) + 1);
}

FT_TEST(test_pf_printf_null_format_sets_errno, "pf_printf null format guard updates errno")
{
    ft_errno = ER_SUCCESS;
    typedef int (*t_pf_printf_function)(const char *format, ...);
    t_pf_printf_function printf_function = pf_printf;
    int result = printf_function(ft_nullptr);
    FT_ASSERT_EQ(0, result);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_pf_printf_fd_success_clears_errno, "pf_printf_fd success clears errno")
{
    const char *file_name = "tmp_pf_printf_errno_success.txt";
    int file_descriptor = ::open(file_name, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (file_descriptor < 0)
        return (0);
    ft_errno = FT_EINVAL;
    int printed = pf_printf_fd(file_descriptor, "ok");
    ::close(file_descriptor);
    ::unlink(file_name);
    FT_ASSERT_EQ(2, printed);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_pf_printf_fd_overflow_sets_errno, "pf_printf_fd overflow guard sets errno")
{
    const char *file_name = "tmp_pf_printf_errno_overflow.txt";
    int file_descriptor = ::open(file_name, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (file_descriptor < 0)
        return (0);
    pf_printf_set_count_override_hook(pf_test_count_overflow_hook);
    ft_errno = ER_SUCCESS;
    int printed = pf_printf_fd(file_descriptor, "x");
    pf_printf_set_count_override_hook(ft_nullptr);
    ::close(file_descriptor);
    ::unlink(file_name);
    FT_ASSERT_EQ(-1, printed);
    FT_ASSERT_EQ(FT_ERANGE, ft_errno);
    return (1);
}

int test_pf_printf_basic(void)
{
    const char *fname = "tmp_pf_printf_basic.txt";
    int fd = ::open(fname, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd < 0)
        return (0);
    pf_printf_fd(fd, "Hello %s %d!", "world", 42);
    ::lseek(fd, 0, SEEK_SET);
    char buf[64];
    ssize_t r = ::read(fd, buf, sizeof(buf) - 1);
    ::close(fd);
    ::unlink(fname);
    if (r < 0)
        return (0);
    buf[r] = '\0';
    return (ft_strcmp(buf, "Hello world 42!") == 0);
}

int test_pf_printf_misc(void)
{
    const char *fname = "tmp_pf_printf_misc.txt";
    int fd = ::open(fname, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd < 0)
        return (0);
    pf_printf_fd(fd, "%c %X %b %%", 'A', 0x2A, 0);
    ::lseek(fd, 0, SEEK_SET);
    char buf[64];
    ssize_t r = ::read(fd, buf, sizeof(buf) - 1);
    ::close(fd);
    ::unlink(fname);
    if (r < 0)
        return (0);
    buf[r] = '\0';
    return (ft_strcmp(buf, "A 2A false %") == 0);
}
int test_pf_printf_bool(void)
{
    const char *fname = "tmp_pf_printf_bool.txt";
    int fd = ::open(fname, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd < 0)
        return (0);
    pf_printf_fd(fd, "%b %b", 1, 0);
    ::lseek(fd, 0, SEEK_SET);
    char buf[64];
    ssize_t r = ::read(fd, buf, sizeof(buf) - 1);
    ::close(fd);
    ::unlink(fname);
    if (r < 0)
        return (0);
    buf[r] = '\0';
    return (ft_strcmp(buf, "true false") == 0);
}

int test_pf_printf_nullptr(void)
{
    const char *fname = "tmp_pf_printf_nullptr.txt";
    int fd = ::open(fname, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd < 0)
        return (0);
    pf_printf_fd(fd, "%p %s", static_cast<void*>(ft_nullptr), static_cast<char*>(ft_nullptr));
    ::lseek(fd, 0, SEEK_SET);
    char buf[64];
    ssize_t r = ::read(fd, buf, sizeof(buf) - 1);
    ::close(fd);
    ::unlink(fname);
    if (r < 0)
        return (0);
    buf[r] = '\0';
    return (ft_strcmp(buf, "0x0 (null)") == 0);
}

int test_pf_printf_modifiers(void)
{
    const char *fname = "tmp_pf_printf_modifiers.txt";
    int fd = ::open(fname, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd < 0)
        return (0);
    long lval = 2147483648L;
    size_t zval = static_cast<size_t>(0x1FFFFFFFFULL);
    pf_printf_fd(fd, "%ld %lu %lx %zu %zx", lval, lval, lval, zval, zval);
    ::lseek(fd, 0, SEEK_SET);
    char buf[128];
    ssize_t r = ::read(fd, buf, sizeof(buf) - 1);
    ::close(fd);
    ::unlink(fname);
    if (r < 0)
        return (0);
    buf[r] = '\0';
    return (ft_strcmp(buf, "2147483648 2147483648 80000000 8589934591 1ffffffff") == 0);
}

int test_pf_printf_float_positive(void)
{
    const char *file_name = "tmp_pf_printf_float_positive.txt";
    int file_descriptor = ::open(file_name, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (file_descriptor < 0)
        return (0);
    pf_printf_fd(file_descriptor, "%.2f", 3.14159);
    ::lseek(file_descriptor, 0, SEEK_SET);
    char buffer[64];
    ssize_t read_bytes = ::read(file_descriptor, buffer, sizeof(buffer) - 1);
    ::close(file_descriptor);
    ::unlink(file_name);
    if (read_bytes < 0)
        return (0);
    buffer[read_bytes] = '\0';
    return (ft_strcmp(buffer, "3.14") == 0);
}

int test_pf_printf_float_negative(void)
{
    const char *file_name = "tmp_pf_printf_float_negative.txt";
    int file_descriptor = ::open(file_name, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (file_descriptor < 0)
        return (0);
    pf_printf_fd(file_descriptor, "%.3f", -2.71828);
    ::lseek(file_descriptor, 0, SEEK_SET);
    char buffer[64];
    ssize_t read_bytes = ::read(file_descriptor, buffer, sizeof(buffer) - 1);
    ::close(file_descriptor);
    ::unlink(file_name);
    if (read_bytes < 0)
        return (0);
    buffer[read_bytes] = '\0';
    return (ft_strcmp(buffer, "-2.718") == 0);
}

int test_pf_printf_float_zero(void)
{
    const char *file_name = "tmp_pf_printf_float_zero.txt";
    int file_descriptor = ::open(file_name, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (file_descriptor < 0)
        return (0);
    pf_printf_fd(file_descriptor, "%.4f", 0.0);
    ::lseek(file_descriptor, 0, SEEK_SET);
    char buffer[64];
    ssize_t read_bytes = ::read(file_descriptor, buffer, sizeof(buffer) - 1);
    ::close(file_descriptor);
    ::unlink(file_name);
    if (read_bytes < 0)
        return (0);
    buffer[read_bytes] = '\0';
    return (ft_strcmp(buffer, "0.0000") == 0);
}

int test_pf_snprintf_basic(void)
{
    char buffer[64];
    pf_snprintf(buffer, sizeof(buffer), "%s %d", "number", 7);
    return (ft_strcmp(buffer, "number 7") == 0);
}
