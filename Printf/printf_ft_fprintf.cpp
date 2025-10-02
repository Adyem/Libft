
#include "printf.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include <cstdio>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <string>
#include <errno.h>

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

static void set_stream_error(void)
{
    int saved_errno;

    saved_errno = errno;
    if (saved_errno != 0)
    {
        ft_errno = saved_errno + ERRNO_OFFSET;
        return ;
    }
    ft_errno = FT_EIO;
    return ;
}

static void write_buffer_stream(const char *buffer, size_t length, FILE *stream, size_t *count)
{
    if (count_has_error(count))
        return ;
    if (length == 0)
        return ;
    size_t written = fwrite(buffer, 1, length, stream);
    if (written != length)
    {
        mark_count_error(count);
        set_stream_error();
        return ;
    }
    *count += written;
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
        { \
            ft_errno = FT_EIO; \
            return (-1); \
        } \
        output.clear(); \
        output.resize(static_cast<size_t>(required_length) + 1); \
        int written_length = std::snprintf(&output[0], output.size(), literal, precision, number); \
        if (written_length < 0) \
        { \
            ft_errno = FT_EIO; \
            return (-1); \
        } \
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

typedef enum
{
    LEN_NONE,
    LEN_L,
    LEN_Z
} LengthModifier;

static void ft_putchar_stream(const char character, FILE *stream, size_t *count)
{
    if (count_has_error(count))
        return ;
    if (fputc(static_cast<unsigned char>(character), stream) == EOF)
    {
        mark_count_error(count);
        set_stream_error();
        return ;
    }
    (*count)++;
}

static void ft_putstr_stream(const char *string, FILE *stream, size_t *count)
{
    if (count_has_error(count))
        return ;
    if (!string)
    {
        write_buffer_stream("(null)", 6, stream, count);
        return ;
    }
    size_t length = ft_strlen(string);
    write_buffer_stream(string, length, stream, count);
}

static void ft_putnbr_stream_recursive(long number, FILE *stream, size_t *count)
{
    if (count_has_error(count))
        return ;
    if (number < 0)
    {
        ft_putchar_stream('-', stream, count);
        if (count_has_error(count))
            return ;
        number = -number;
    }
    if (number >= 10)
        ft_putnbr_stream_recursive(number / 10, stream, count);
    if (count_has_error(count))
        return ;
    ft_putchar_stream(static_cast<char>('0' + (number % 10)), stream, count);
}

static void ft_putnbr_stream(long number, FILE *stream, size_t *count)
{
    ft_putnbr_stream_recursive(number, stream, count);
}

static void ft_putunsigned_stream_recursive(uintmax_t number, FILE *stream, size_t *count)
{
    if (count_has_error(count))
        return ;
    if (number >= 10)
        ft_putunsigned_stream_recursive(number / 10, stream, count);
    if (count_has_error(count))
        return ;
    ft_putchar_stream(static_cast<char>('0' + (number % 10)), stream, count);
}

static void ft_putunsigned_stream(uintmax_t number, FILE *stream, size_t *count)
{
    ft_putunsigned_stream_recursive(number, stream, count);
}

static void ft_puthex_stream_recursive(uintmax_t number, FILE *stream, bool uppercase, size_t *count)
{
    if (count_has_error(count))
        return ;
    if (number >= 16)
        ft_puthex_stream_recursive(number / 16, stream, uppercase, count);
    if (count_has_error(count))
        return ;
    uintmax_t digit = number % 16;
    char character;
    if (digit < 10)
        character = static_cast<char>('0' + digit);
    else
    {
        char base_char;
        if (uppercase)
            base_char = 'A';
        else
            base_char = 'a';
        character = static_cast<char>(base_char + (digit - 10));
    }
    ft_putchar_stream(character, stream, count);
}

static void ft_puthex_stream(uintmax_t number, FILE *stream, bool uppercase, size_t *count)
{
    ft_puthex_stream_recursive(number, stream, uppercase, count);
}

static void ft_putoctal_stream_recursive(uintmax_t number, FILE *stream, size_t *count)
{
    if (count_has_error(count))
        return ;
    if (number >= 8)
        ft_putoctal_stream_recursive(number / 8, stream, count);
    if (count_has_error(count))
        return ;
    ft_putchar_stream(static_cast<char>('0' + (number % 8)), stream, count);
}

static void ft_putoctal_stream(uintmax_t number, FILE *stream, size_t *count)
{
    ft_putoctal_stream_recursive(number, stream, count);
}

static void ft_putptr_stream(void *pointer, FILE *stream, size_t *count)
{
    if (count_has_error(count))
        return ;
    ft_putstr_stream("0x", stream, count);
    if (count_has_error(count))
        return ;
    ft_puthex_stream(reinterpret_cast<uintptr_t>(pointer), stream, false, count);
}

static void ft_putfloat_stream(double number, FILE *stream, size_t *count, int precision)
{
    if (count_has_error(count))
        return ;
    std::string formatted_output;
    if (format_double_output('f', precision, number, formatted_output) != 0)
    {
        mark_count_error(count);
        set_stream_error();
        return ;
    }
    size_t output_length = formatted_output.length();
    if (output_length == 0)
        return ;
    write_buffer_stream(formatted_output.c_str(), output_length, stream, count);
}

static void ft_putscientific_stream(double number, bool uppercase, FILE *stream, size_t *count, int precision)
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
        set_stream_error();
        return ;
    }
    size_t output_length = formatted_output.length();
    if (output_length == 0)
        return ;
    write_buffer_stream(formatted_output.c_str(), output_length, stream, count);
}

static void ft_putgeneral_stream(double number, bool uppercase, FILE *stream, size_t *count, int precision)
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
        set_stream_error();
        return ;
    }
    size_t output_length = formatted_output.length();
    if (output_length == 0)
        return ;
    write_buffer_stream(formatted_output.c_str(), output_length, stream, count);
}

int ft_vfprintf(FILE *stream, const char *format, va_list args)
{
    if (stream == ft_nullptr || format == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (-1);
    }
    size_t count = 0;
    size_t index = 0;
    while (format[index])
    {
        if (count == SIZE_MAX)
            break ;
        if (format[index] == '%')
        {
            index++;
            if (format[index] == '\0')
                break;
            LengthModifier len_mod = LEN_NONE;
            if (format[index] == 'l')
            {
                len_mod = LEN_L;
                index++;
            }
            else if (format[index] == 'z')
            {
                len_mod = LEN_Z;
                index++;
            }
            char spec = format[index];
            if (spec == '\0')
                break;
            if (spec == 'c')
            {
                char character = static_cast<char>(va_arg(args, int));
                ft_putchar_stream(character, stream, &count);
            }
            else if (spec == 's')
            {
                char *string = va_arg(args, char *);
                ft_putstr_stream(string, stream, &count);
            }
            else if (spec == 'd' || spec == 'i')
            {
                if (len_mod == LEN_L)
                {
                    long number = va_arg(args, long);
                    ft_putnbr_stream(number, stream, &count);
                }
                else
                {
                    int number = va_arg(args, int);
                    ft_putnbr_stream(number, stream, &count);
                }
            }
            else if (spec == 'u')
            {
                if (len_mod == LEN_L)
                {
                    uintmax_t number = va_arg(args, unsigned long);
                    ft_putunsigned_stream(number, stream, &count);
                }
                else if (len_mod == LEN_Z)
                {
                    size_t number = va_arg(args, size_t);
                    ft_putunsigned_stream(number, stream, &count);
                }
                else
                {
                    unsigned int number = va_arg(args, unsigned int);
                    ft_putunsigned_stream(number, stream, &count);
                }
            }
            else if (spec == 'x' || spec == 'X')
            {
                bool uppercase = (spec == 'X');
                if (len_mod == LEN_L)
                {
                    uintmax_t number = va_arg(args, unsigned long);
                    ft_puthex_stream(number, stream, uppercase, &count);
                }
                else if (len_mod == LEN_Z)
                {
                    size_t number = va_arg(args, size_t);
                    ft_puthex_stream(number, stream, uppercase, &count);
                }
                else
                {
                    unsigned int number = va_arg(args, unsigned int);
                    ft_puthex_stream(number, stream, uppercase, &count);
                }
            }
            else if (spec == 'o')
            {
                if (len_mod == LEN_L)
                {
                    uintmax_t number = va_arg(args, unsigned long);
                    ft_putoctal_stream(number, stream, &count);
                }
                else if (len_mod == LEN_Z)
                {
                    size_t number = va_arg(args, size_t);
                    ft_putoctal_stream(number, stream, &count);
                }
                else
                {
                    unsigned int number = va_arg(args, unsigned int);
                    ft_putoctal_stream(number, stream, &count);
                }
            }
            else if (spec == 'f')
            {
                double number = va_arg(args, double);
                ft_putfloat_stream(number, stream, &count, 6);
            }
            else if (spec == 'e' || spec == 'E')
            {
                bool uppercase = (spec == 'E');
                double number = va_arg(args, double);
                ft_putscientific_stream(number, uppercase, stream, &count, 6);
            }
            else if (spec == 'g' || spec == 'G')
            {
                bool uppercase = (spec == 'G');
                double number = va_arg(args, double);
                ft_putgeneral_stream(number, uppercase, stream, &count, 6);
            }
            else if (spec == 'p')
            {
                void *pointer = va_arg(args, void *);
                ft_putptr_stream(pointer, stream, &count);
            }
            else if (spec == 'b')
            {
                int boolean_value = va_arg(args, int);
                if (boolean_value)
                    ft_putstr_stream("true", stream, &count);
                else
                    ft_putstr_stream("false", stream, &count);
            }
            else if (spec == 'n')
            {
                if (count != SIZE_MAX)
                {
                    if (len_mod == LEN_L)
                    {
                        long *out = va_arg(args, long *);
                        if (out)
                            *out = static_cast<long>(count);
                    }
                    else if (len_mod == LEN_Z)
                    {
                        size_t *out = va_arg(args, size_t *);
                        if (out)
                            *out = count;
                    }
                    else
                    {
                        int *out = va_arg(args, int *);
                        if (out)
                            *out = static_cast<int>(count);
                    }
                }
            }
            else if (spec == '%')
            {
                ft_putchar_stream('%', stream, &count);
            }
            else
            {
                ft_putchar_stream('%', stream, &count);
                ft_putchar_stream(spec, stream, &count);
            }
        }
        else
        {
            ft_putchar_stream(format[index], stream, &count);
        }
        index++;
    }
    if (count != SIZE_MAX)
    {
        if (fflush(stream) == EOF)
        {
            mark_count_error(&count);
            set_stream_error();
        }
    }
    if (count == SIZE_MAX)
        return (-1);
    if (count > static_cast<size_t>(INT_MAX))
    {
        ft_errno = FT_ERANGE;
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (static_cast<int>(count));
}

int ft_fprintf(FILE *stream, const char *format, ...)
{
    if (stream == ft_nullptr || format == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (-1);
    }
    va_list args;
    va_start(args, format);
    int printed = ft_vfprintf(stream, format, args);
    va_end(args);
    return (printed);
}

