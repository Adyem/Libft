#include "../Printf/printf.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/nullptr.hpp"
#include <fcntl.h>
#include <unistd.h>

int test_pf_printf_basic(void)
{
    const char *fname = "tmp_pf_printf_basic.txt";
    int fd = ::open(fname, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd < 0)
        return 0;
    pf_printf_fd(fd, "Hello %s %d!", "world", 42);
    ::lseek(fd, 0, SEEK_SET);
    char buf[64];
    ssize_t r = ::read(fd, buf, sizeof(buf) - 1);
    ::close(fd);
    ::unlink(fname);
    if (r < 0)
        return 0;
    buf[r] = '\0';
    return (ft_strcmp(buf, "Hello world 42!") == 0);
}

int test_pf_printf_misc(void)
{
    const char *fname = "tmp_pf_printf_misc.txt";
    int fd = ::open(fname, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd < 0)
        return 0;
    pf_printf_fd(fd, "%c %X %b %%", 'A', 0x2A, 0);
    ::lseek(fd, 0, SEEK_SET);
    char buf[64];
    ssize_t r = ::read(fd, buf, sizeof(buf) - 1);
    ::close(fd);
    ::unlink(fname);
    if (r < 0)
        return 0;
    buf[r] = '\0';
    return (ft_strcmp(buf, "A 2A false %") == 0);
}
int test_pf_printf_bool(void)
{
    const char *fname = "tmp_pf_printf_bool.txt";
    int fd = ::open(fname, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd < 0)
        return 0;
    pf_printf_fd(fd, "%b %b", 1, 0);
    ::lseek(fd, 0, SEEK_SET);
    char buf[64];
    ssize_t r = ::read(fd, buf, sizeof(buf) - 1);
    ::close(fd);
    ::unlink(fname);
    if (r < 0)
        return 0;
    buf[r] = '\0';
    return (ft_strcmp(buf, "true false") == 0);
}

int test_pf_printf_nullptr(void)
{
    const char *fname = "tmp_pf_printf_nullptr.txt";
    int fd = ::open(fname, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd < 0)
        return 0;
    pf_printf_fd(fd, "%p %s", static_cast<void*>(ft_nullptr), static_cast<char*>(ft_nullptr));
    ::lseek(fd, 0, SEEK_SET);
    char buf[64];
    ssize_t r = ::read(fd, buf, sizeof(buf) - 1);
    ::close(fd);
    ::unlink(fname);
    if (r < 0)
        return 0;
    buf[r] = '\0';
    return (ft_strcmp(buf, "0x0 (null)") == 0);
}

int test_pf_printf_modifiers(void)
{
    const char *fname = "tmp_pf_printf_modifiers.txt";
    int fd = ::open(fname, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd < 0)
        return 0;
    long lval = 2147483648L;
    size_t zval = static_cast<size_t>(0x1FFFFFFFFULL);
    pf_printf_fd(fd, "%ld %lu %lx %zu %zx", lval, lval, lval, zval, zval);
    ::lseek(fd, 0, SEEK_SET);
    char buf[128];
    ssize_t r = ::read(fd, buf, sizeof(buf) - 1);
    ::close(fd);
    ::unlink(fname);
    if (r < 0)
        return 0;
    buf[r] = '\0';
    return (ft_strcmp(buf, "2147483648 2147483648 80000000 8589934591 1ffffffff") == 0);
}
