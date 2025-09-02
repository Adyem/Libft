// Custom implementation of vfprintf-style formatting for FILE streams
#include "printf.hpp"
#include "../CPP_class/nullptr.hpp"
#include <cstdio>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

typedef enum
{
    LEN_NONE,
    LEN_L,
    LEN_Z
} LengthModifier;

static void ft_putchar_stream(const char character, FILE *stream, size_t *count)
{
    fputc(character, stream);
    (*count)++;
}

static void ft_putstr_stream(const char *string, FILE *stream, size_t *count)
{
    if (!string)
    {
        fwrite("(null)", 1, 6, stream);
        *count += 6;
        return;
    }
    size_t length = strlen(string);
    fwrite(string, 1, length, stream);
    *count += length;
}

static void ft_putnbr_stream_recursive(long number, FILE *stream, size_t *count)
{
    if (number < 0)
    {
        ft_putchar_stream('-', stream, count);
        number = -number;
    }
    if (number >= 10)
        ft_putnbr_stream_recursive(number / 10, stream, count);
    ft_putchar_stream(static_cast<char>('0' + (number % 10)), stream, count);
}

static void ft_putnbr_stream(long number, FILE *stream, size_t *count)
{
    ft_putnbr_stream_recursive(number, stream, count);
}

static void ft_putunsigned_stream_recursive(uintmax_t number, FILE *stream, size_t *count)
{
    if (number >= 10)
        ft_putunsigned_stream_recursive(number / 10, stream, count);
    ft_putchar_stream(static_cast<char>('0' + (number % 10)), stream, count);
}

static void ft_putunsigned_stream(uintmax_t number, FILE *stream, size_t *count)
{
    ft_putunsigned_stream_recursive(number, stream, count);
}

static void ft_puthex_stream_recursive(uintmax_t number, FILE *stream, bool uppercase, size_t *count)
{
    if (number >= 16)
        ft_puthex_stream_recursive(number / 16, stream, uppercase, count);
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
    if (number >= 8)
        ft_putoctal_stream_recursive(number / 8, stream, count);
    ft_putchar_stream(static_cast<char>('0' + (number % 8)), stream, count);
}

static void ft_putoctal_stream(uintmax_t number, FILE *stream, size_t *count)
{
    ft_putoctal_stream_recursive(number, stream, count);
}

static void ft_putptr_stream(void *pointer, FILE *stream, size_t *count)
{
    ft_putstr_stream("0x", stream, count);
    ft_puthex_stream(reinterpret_cast<uintptr_t>(pointer), stream, false, count);
}

static void ft_putfloat_stream(double number, FILE *stream, size_t *count)
{
    if (number < 0)
    {
        ft_putchar_stream('-', stream, count);
        number = -number;
    }
    long integer_part = static_cast<long>(number);
    ft_putnbr_stream(integer_part, stream, count);
    ft_putchar_stream('.', stream, count);
    double fractional = number - static_cast<double>(integer_part);
    for (int i = 0; i < 6; ++i)
    {
        fractional *= 10;
        int digit = static_cast<int>(fractional);
        ft_putchar_stream(static_cast<char>('0' + digit), stream, count);
        fractional -= digit;
    }
}

static void ft_putscientific_stream(double number, bool uppercase, FILE *stream, size_t *count)
{
    if (number == 0.0)
    {
        if (uppercase)
            ft_putstr_stream("0.000000E+00", stream, count);
        else
            ft_putstr_stream("0.000000e+00", stream, count);
        return;
    }
    if (number < 0)
    {
        ft_putchar_stream('-', stream, count);
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
    ft_putchar_stream(static_cast<char>('0' + integer_part), stream, count);
    ft_putchar_stream('.', stream, count);
    double fractional = number - static_cast<double>(integer_part);
    for (int i = 0; i < 6; ++i)
    {
        fractional *= 10.0;
        int digit = static_cast<int>(fractional);
        ft_putchar_stream(static_cast<char>('0' + digit), stream, count);
        fractional -= digit;
    }
    ft_putchar_stream(uppercase ? 'E' : 'e', stream, count);
    if (exponent >= 0)
        ft_putchar_stream('+', stream, count);
    else
    {
        ft_putchar_stream('-', stream, count);
        exponent = -exponent;
    }
    if (exponent >= 100)
        ft_putnbr_stream(exponent, stream, count);
    else
    {
        ft_putchar_stream(static_cast<char>('0' + (exponent / 10)), stream, count);
        ft_putchar_stream(static_cast<char>('0' + (exponent % 10)), stream, count);
    }
}

static void ft_putgeneral_stream(double number, bool uppercase, FILE *stream, size_t *count)
{
    if (number == 0.0)
    {
        ft_putfloat_stream(0.0, stream, count);
        return;
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
        ft_putscientific_stream(number, uppercase, stream, count);
    else
        ft_putfloat_stream(number, stream, count);
}

int ft_vfprintf(FILE *stream, const char *format, va_list args)
{
    if (stream == ft_nullptr || format == ft_nullptr)
        return (0);
    size_t count = 0;
    size_t index = 0;
    while (format[index])
    {
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
                ft_putfloat_stream(number, stream, &count);
            }
            else if (spec == 'e' || spec == 'E')
            {
                bool uppercase = (spec == 'E');
                double number = va_arg(args, double);
                ft_putscientific_stream(number, uppercase, stream, &count);
            }
            else if (spec == 'g' || spec == 'G')
            {
                bool uppercase = (spec == 'G');
                double number = va_arg(args, double);
                ft_putgeneral_stream(number, uppercase, stream, &count);
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
    return (static_cast<int>(count));
}

int ft_fprintf(FILE *stream, const char *format, ...)
{
    if (stream == ft_nullptr || format == ft_nullptr)
        return (0);
    va_list args;
    va_start(args, format);
    int printed = ft_vfprintf(stream, format, args);
    va_end(args);
    return (printed);
}

