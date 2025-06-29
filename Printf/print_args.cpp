#include "printf_internal.hpp"
#include <cstdarg>
#include <unistd.h>
#include "../Linux/linux_file.hpp"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <stddef.h>

static inline ssize_t ft_platform_write(int fd, const char *s, size_t len)
{
#ifdef _WIN32
    return (ft_write(fd, s, static_cast<unsigned int>(len)));
#else
    return (ft_write(fd, s, len));
#endif
}

size_t ft_strlen_printf(const char *s)
{
    size_t len = 0;
    if (!s)
        return (6);
    while (s[len])
        len++;
    return (len);
}

void ft_putchar_fd(const char c, int fd, size_t *count)
{
    ssize_t return_value = ft_write(fd, &c, 1);
    (void)return_value;
    (*count)++;
    return ;
}

void ft_putstr_fd(const char *s, int fd, size_t *count)
{
	ssize_t return_value;
    if (!s)
    {
        return_value = ft_write(fd, "(null)", 6);
        *count += 6;
        return ;
    }
    size_t len = ft_strlen_printf(s);
    return_value = ft_platform_write(fd, s, len);
    *count += len;
    (void)return_value;
    return ;
}

void ft_putnbr_fd_recursive(long n, int fd, size_t *count)
{
    char c;
    if (n < 0)
    {
        ft_putchar_fd('-', fd, count);
        n = -n;
    }
    if (n >= 10)
        ft_putnbr_fd_recursive(n / 10, fd, count);
    c = static_cast<char>('0' + (n % 10));
    ft_putchar_fd(c, fd, count);
    return ;
}

void ft_putnbr_fd(long n, int fd, size_t *count)
{
    ft_putnbr_fd_recursive(n, fd, count);
    return ;
}

void ft_putunsigned_fd_recursive(uintmax_t n, int fd, size_t *count)
{
    char c;
    if (n >= 10)
        ft_putunsigned_fd_recursive(n / 10, fd, count);
    c = static_cast<char>('0' + (n % 10));
    ft_putchar_fd(c, fd, count);
    return ;
}

void ft_putunsigned_fd(uintmax_t n, int fd, size_t *count)
{
    ft_putunsigned_fd_recursive(n, fd, count);
    return ;
}

void ft_puthex_fd_recursive(uintmax_t n, int fd, bool uppercase, size_t *count)
{
    char c;

    if (n >= 16)
        ft_puthex_fd_recursive(n / 16, fd, uppercase, count);
    if ((n % 16) < 10)
        c = '0' + (n % 16);
    else
    {
        if (uppercase)
            c = 'A' + ((n % 16) - 10);
        else
            c = 'a' + ((n % 16) - 10);
    }
    ft_putchar_fd(c, fd, count);
    return ;
}

void ft_puthex_fd(uintmax_t n, int fd, bool uppercase, size_t *count)
{
    ft_puthex_fd_recursive(n, fd, uppercase, count);
    return ;
}

void ft_putptr_fd(void *ptr, int fd, size_t *count)
{
    uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
    ft_putstr_fd("0x", fd, count);
    ft_puthex_fd(addr, fd, false, count);
    return ;
}
