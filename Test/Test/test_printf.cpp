#include "../../Printf/printf.hpp"
#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include <fcntl.h>
#include <unistd.h>

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
