#include "printf_internal.hpp"
#include "../Libft/libft.hpp"
#include <cstdarg>
#include <unistd.h>
#include "../System_utils/system_utils.hpp"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <stddef.h>
#include <string>
#include <cstdio>

static bool count_has_error(size_t *count)
{
    if (!count)
        return (true);
    if (*count == SIZE_MAX)
        return (true);
    return (false);
}

static void mark_count_error(size_t *count)
{
    if (!count)
        return ;
    *count = SIZE_MAX;
    return ;
}

static void write_buffer_fd(const char *buffer, size_t length, int fd, size_t *count)
{
    if (count_has_error(count))
        return ;
    if (length == 0)
        return ;
    ssize_t return_value = su_write(fd, buffer, length);
    if (return_value != static_cast<ssize_t>(length))
    {
        mark_count_error(count);
        return ;
    }
    *count += length;
    return ;
}

static int format_double_output(char specifier, int precision, double number, std::string &output)
{
    if (precision < 0)
        precision = 6;
    if ((specifier == 'g' || specifier == 'G') && precision == 0)
        precision = 1;
#define FORMAT_DOUBLE_CASE(character, literal) \
    if (specifier == character) \
    { \
        int required_length = std::snprintf(ft_nullptr, 0, literal, precision, number); \
        if (required_length < 0) \
            return (-1); \
        output.clear(); \
        output.resize(static_cast<size_t>(required_length) + 1); \
        int written_length = std::snprintf(&output[0], output.size(), literal, precision, number); \
        if (written_length < 0) \
            return (-1); \
        output.resize(static_cast<size_t>(written_length)); \
        return (0); \
    }

    FORMAT_DOUBLE_CASE('f', "%.*f");
    FORMAT_DOUBLE_CASE('F', "%.*F");
    FORMAT_DOUBLE_CASE('e', "%.*e");
    FORMAT_DOUBLE_CASE('E', "%.*E");
    FORMAT_DOUBLE_CASE('g', "%.*g");
    FORMAT_DOUBLE_CASE('G', "%.*G");
#undef FORMAT_DOUBLE_CASE
    return (-1);
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
    write_buffer_fd(&character, 1, fd, count);
    return ;
}

void ft_putstr_fd(const char *string, int fd, size_t *count)
{
    if (count_has_error(count))
        return ;
    if (!string)
    {
        write_buffer_fd("(null)", 6, fd, count);
        return ;
    }
    size_t length = ft_strlen_printf(string);
    write_buffer_fd(string, length, fd, count);
    return ;
}

void ft_putnbr_fd_recursive(long number, int fd, size_t *count)
{
    char character;

    if (count_has_error(count))
        return ;
    if (number < 0)
    {
        ft_putchar_fd('-', fd, count);
        if (count_has_error(count))
            return ;
        number = -number;
    }
    if (number >= 10)
        ft_putnbr_fd_recursive(number / 10, fd, count);
    if (count_has_error(count))
        return ;
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

    if (count_has_error(count))
        return ;
    if (number >= 10)
        ft_putunsigned_fd_recursive(number / 10, fd, count);
    if (count_has_error(count))
        return ;
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

    if (count_has_error(count))
        return ;
    if (number >= 16)
        ft_puthex_fd_recursive(number / 16, fd, uppercase, count);
    if (count_has_error(count))
        return ;
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

    if (count_has_error(count))
        return ;
    if (number >= 8)
        ft_putoctal_fd_recursive(number / 8, fd, count);
    if (count_has_error(count))
        return ;
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
    if (count_has_error(count))
        return ;
    uintptr_t address = reinterpret_cast<uintptr_t>(pointer);
    ft_putstr_fd("0x", fd, count);
    if (count_has_error(count))
        return ;
    ft_puthex_fd(address, fd, false, count);
    return ;
}

void ft_putfloat_fd(double number, int fd, size_t *count, int precision)
{
    if (count_has_error(count))
        return ;
    std::string formatted_output;
    if (format_double_output('f', precision, number, formatted_output) != 0)
    {
        mark_count_error(count);
        return ;
    }
    size_t output_length = formatted_output.length();
    if (output_length == 0)
        return ;
    write_buffer_fd(formatted_output.c_str(), output_length, fd, count);
    return ;
}

void ft_putscientific_fd(double number, bool uppercase, int fd, size_t *count, int precision)
{
    if (count_has_error(count))
        return ;
    char specifier;

    if (uppercase)
        specifier = 'E';
    else
        specifier = 'e';
    std::string formatted_output;
    if (format_double_output(specifier, precision, number, formatted_output) != 0)
    {
        mark_count_error(count);
        return ;
    }
    size_t output_length = formatted_output.length();
    if (output_length == 0)
        return ;
    write_buffer_fd(formatted_output.c_str(), output_length, fd, count);
    return ;
}

void ft_putgeneral_fd(double number, bool uppercase, int fd, size_t *count, int precision)
{
    if (count_has_error(count))
        return ;
    char specifier;

    if (uppercase)
        specifier = 'G';
    else
        specifier = 'g';
    std::string formatted_output;
    if (format_double_output(specifier, precision, number, formatted_output) != 0)
    {
        mark_count_error(count);
        return ;
    }
    size_t output_length = formatted_output.length();
    if (output_length == 0)
        return ;
    write_buffer_fd(formatted_output.c_str(), output_length, fd, count);
    return ;
}
