#include "printf_internal.hpp"
#include "../Libft/libft.hpp"
#include "../Math/math.hpp"
#include <cstdarg>
#include <unistd.h>
#include "../System_utils/system_utils.hpp"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <stddef.h>
#include <cfloat>

static inline ssize_t su_platform_write(int fd, const char *string, size_t length)
{
#ifdef _WIN32
    return (su_write(fd, string, static_cast<unsigned int>(length)));
#else
    return (su_write(fd, string, length));
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
    ssize_t return_value = su_write(fd, &character, 1);
    (void)return_value;
    (*count)++;
    return ;
}

void ft_putstr_fd(const char *string, int fd, size_t *count)
{
    ssize_t return_value;
    if (!string)
    {
        return_value = su_write(fd, "(null)", 6);
        *count += 6;
        return ;
    }
    size_t length = ft_strlen_printf(string);
    return_value = su_platform_write(fd, string, length);
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

void ft_putoctal_fd_recursive(uintmax_t number, int fd, size_t *count)
{
    char character;

    if (number >= 8)
        ft_putoctal_fd_recursive(number / 8, fd, count);
    character = static_cast<char>('0' + (number % 8));
    ft_putchar_fd(character, fd, count);
    return ;
}

void ft_putoctal_fd(uintmax_t number, int fd, size_t *count)
{
    ft_putoctal_fd_recursive(number, fd, count);
    return ;
}

void ft_putptr_fd(void *pointer, int fd, size_t *count)
{
    uintptr_t address = reinterpret_cast<uintptr_t>(pointer);
    ft_putstr_fd("0x", fd, count);
    ft_puthex_fd(address, fd, false, count);
    return ;
}

void ft_putfloat_fd(double number, int fd, size_t *count)
{
    if (number < 0)
    {
        ft_putchar_fd('-', fd, count);
        number = -number;
    }
    long integer_part = static_cast<long>(number);
    ft_putnbr_fd(integer_part, fd, count);
    ft_putchar_fd('.', fd, count);
    double fractional = number - static_cast<double>(integer_part);
    int index_fraction = 0;
    while (index_fraction < 6)
    {
        fractional *= 10;
        int digit = static_cast<int>(fractional);
        ft_putchar_fd(static_cast<char>('0' + digit), fd, count);
        fractional -= digit;
        index_fraction++;
    }
    return ;
}

void ft_putscientific_fd(double number, bool uppercase, int fd, size_t *count)
{
    if (math_fabs(number) <= DBL_EPSILON)
    {
        if (uppercase)
            ft_putstr_fd("0.000000E+00", fd, count);
        else
            ft_putstr_fd("0.000000e+00", fd, count);
        return ;
    }
    if (number < 0)
    {
        ft_putchar_fd('-', fd, count);
        number = -number;
    }
    int exponent = 0;
    while (number >= 10.0)
    {
        number /= 10.0;
        exponent++;
    }
    while (number < 1.0)
    {
        number *= 10.0;
        exponent--;
    }
    long integer_part = static_cast<long>(number);
    ft_putchar_fd(static_cast<char>('0' + integer_part), fd, count);
    ft_putchar_fd('.', fd, count);
    double fractional = number - static_cast<double>(integer_part);
    int index_fraction = 0;
    while (index_fraction < 6)
    {
        fractional *= 10.0;
        int digit = static_cast<int>(fractional);
        ft_putchar_fd(static_cast<char>('0' + digit), fd, count);
        fractional -= digit;
        index_fraction++;
    }
    char exponent_character;
    if (uppercase)
        exponent_character = 'E';
    else
        exponent_character = 'e';
    ft_putchar_fd(exponent_character, fd, count);
    if (exponent >= 0)
        ft_putchar_fd('+', fd, count);
    else
    {
        ft_putchar_fd('-', fd, count);
        exponent = -exponent;
    }
    if (exponent >= 100)
        ft_putnbr_fd(exponent, fd, count);
    else
    {
        ft_putchar_fd(static_cast<char>('0' + (exponent / 10)), fd, count);
        ft_putchar_fd(static_cast<char>('0' + (exponent % 10)), fd, count);
    }
    return ;
}

void ft_putgeneral_fd(double number, bool uppercase, int fd, size_t *count)
{
    if (math_fabs(number) <= DBL_EPSILON)
    {
        ft_putfloat_fd(0.0, fd, count);
        return ;
    }
    double temp = number;
    if (temp < 0)
        temp = -temp;
    int exponent = 0;
    while (temp >= 10.0)
    {
        temp /= 10.0;
        exponent++;
    }
    while (temp < 1.0)
    {
        temp *= 10.0;
        exponent--;
    }
    if (exponent < -4 || exponent >= 6)
        ft_putscientific_fd(number, uppercase, fd, count);
    else
        ft_putfloat_fd(number, fd, count);
    return ;
}
