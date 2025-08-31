#include "printf_internal.hpp"
#include <cstdarg>
#include <unistd.h>
#include "../Linux/linux_file.hpp"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <stddef.h>

static inline ssize_t ft_platform_write(int fd, const char *string, size_t length)
{
#ifdef _WIN32
    return (ft_write(fd, string, static_cast<unsigned int>(length)));
#else
    return (ft_write(fd, string, length));
#endif
}

size_t ft_strlen_printf(const char *string)
{
    size_t length = 0;
    if (!string)
        return (6);
    while (string[length])
        length++;
    return (length);
}

void ft_putchar_fd(const char character, int fd, size_t *count)
{
    ssize_t return_value = ft_write(fd, &character, 1);
    (void)return_value;
    (*count)++;
    return ;
}

void ft_putstr_fd(const char *string, int fd, size_t *count)
{
    ssize_t return_value;
    if (!string)
    {
        return_value = ft_write(fd, "(null)", 6);
        *count += 6;
        return ;
    }
    size_t length = ft_strlen_printf(string);
    return_value = ft_platform_write(fd, string, length);
    *count += length;
    (void)return_value;
    return ;
}

void ft_putnbr_fd_recursive(long number, int fd, size_t *count)
{
    char character;
    if (number < 0)
    {
        ft_putchar_fd('-', fd, count);
        number = -number;
    }
    if (number >= 10)
        ft_putnbr_fd_recursive(number / 10, fd, count);
    character = static_cast<char>('0' + (number % 10));
    ft_putchar_fd(character, fd, count);
    return ;
}

void ft_putnbr_fd(long number, int fd, size_t *count)
{
    ft_putnbr_fd_recursive(number, fd, count);
    return ;
}

void ft_putunsigned_fd_recursive(uintmax_t number, int fd, size_t *count)
{
    char character;
    if (number >= 10)
        ft_putunsigned_fd_recursive(number / 10, fd, count);
    character = static_cast<char>('0' + (number % 10));
    ft_putchar_fd(character, fd, count);
    return ;
}

void ft_putunsigned_fd(uintmax_t number, int fd, size_t *count)
{
    ft_putunsigned_fd_recursive(number, fd, count);
    return ;
}

void ft_puthex_fd_recursive(uintmax_t number, int fd, bool uppercase, size_t *count)
{
    char character;

    if (number >= 16)
        ft_puthex_fd_recursive(number / 16, fd, uppercase, count);
    if ((number % 16) < 10)
        character = '0' + (number % 16);
    else
    {
        if (uppercase)
            character = 'A' + ((number % 16) - 10);
        else
            character = 'a' + ((number % 16) - 10);
    }
    ft_putchar_fd(character, fd, count);
    return ;
}

void ft_puthex_fd(uintmax_t number, int fd, bool uppercase, size_t *count)
{
    ft_puthex_fd_recursive(number, fd, uppercase, count);
    return ;
}

void ft_putptr_fd(void *pointer, int fd, size_t *count)
{
    uintptr_t address = reinterpret_cast<uintptr_t>(pointer);
    ft_putstr_fd("0x", fd, count);
    ft_puthex_fd(address, fd, false, count);
    return ;
}
