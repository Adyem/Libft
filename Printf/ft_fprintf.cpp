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

static void ft_putchar_stream(const char c, FILE *stream, size_t *count)
{
    fputc(c, stream);
    (*count)++;
}

static void ft_putstr_stream(const char *s, FILE *stream, size_t *count)
{
    if (!s)
    {
        fwrite("(null)", 1, 6, stream);
        *count += 6;
        return;
    }
    size_t len = strlen(s);
    fwrite(s, 1, len, stream);
    *count += len;
}

static void ft_putnbr_stream_recursive(long n, FILE *stream, size_t *count)
{
    if (n < 0)
    {
        ft_putchar_stream('-', stream, count);
        n = -n;
    }
    if (n >= 10)
        ft_putnbr_stream_recursive(n / 10, stream, count);
    ft_putchar_stream(static_cast<char>('0' + (n % 10)), stream, count);
}

static void ft_putnbr_stream(long n, FILE *stream, size_t *count)
{
    ft_putnbr_stream_recursive(n, stream, count);
}

static void ft_putunsigned_stream_recursive(uintmax_t n, FILE *stream, size_t *count)
{
    if (n >= 10)
        ft_putunsigned_stream_recursive(n / 10, stream, count);
    ft_putchar_stream(static_cast<char>('0' + (n % 10)), stream, count);
}

static void ft_putunsigned_stream(uintmax_t n, FILE *stream, size_t *count)
{
    ft_putunsigned_stream_recursive(n, stream, count);
}

static void ft_puthex_stream_recursive(uintmax_t n, FILE *stream, bool uppercase, size_t *count)
{
    if (n >= 16)
        ft_puthex_stream_recursive(n / 16, stream, uppercase, count);
    uintmax_t digit = n % 16;
    char c;
    if (digit < 10)
        c = static_cast<char>('0' + digit);
    else
        c = static_cast<char>((uppercase ? 'A' : 'a') + (digit - 10));
    ft_putchar_stream(c, stream, count);
}

static void ft_puthex_stream(uintmax_t n, FILE *stream, bool uppercase, size_t *count)
{
    ft_puthex_stream_recursive(n, stream, uppercase, count);
}

static void ft_putptr_stream(void *ptr, FILE *stream, size_t *count)
{
    ft_putstr_stream("0x", stream, count);
    ft_puthex_stream(reinterpret_cast<uintptr_t>(ptr), stream, false, count);
}

int ft_vfprintf(FILE *stream, const char *format, va_list args)
{
    if (stream == ft_nullptr || format == ft_nullptr)
        return (0);
    size_t count = 0;
    size_t i = 0;
    while (format[i])
    {
        if (format[i] == '%')
        {
            i++;
            if (format[i] == '\0')
                break;
            LengthModifier len_mod = LEN_NONE;
            if (format[i] == 'l')
            {
                len_mod = LEN_L;
                i++;
            }
            else if (format[i] == 'z')
            {
                len_mod = LEN_Z;
                i++;
            }
            char spec = format[i];
            if (spec == '\0')
                break;
            if (spec == 'c')
            {
                char c = static_cast<char>(va_arg(args, int));
                ft_putchar_stream(c, stream, &count);
            }
            else if (spec == 's')
            {
                char *s = va_arg(args, char *);
                ft_putstr_stream(s, stream, &count);
            }
            else if (spec == 'd' || spec == 'i')
            {
                if (len_mod == LEN_L)
                {
                    long num = va_arg(args, long);
                    ft_putnbr_stream(num, stream, &count);
                }
                else
                {
                    int num = va_arg(args, int);
                    ft_putnbr_stream(num, stream, &count);
                }
            }
            else if (spec == 'u')
            {
                if (len_mod == LEN_L)
                {
                    uintmax_t num = va_arg(args, unsigned long);
                    ft_putunsigned_stream(num, stream, &count);
                }
                else if (len_mod == LEN_Z)
                {
                    size_t num = va_arg(args, size_t);
                    ft_putunsigned_stream(num, stream, &count);
                }
                else
                {
                    unsigned int num = va_arg(args, unsigned int);
                    ft_putunsigned_stream(num, stream, &count);
                }
            }
            else if (spec == 'x' || spec == 'X')
            {
                bool uppercase = (spec == 'X');
                if (len_mod == LEN_L)
                {
                    uintmax_t num = va_arg(args, unsigned long);
                    ft_puthex_stream(num, stream, uppercase, &count);
                }
                else if (len_mod == LEN_Z)
                {
                    size_t num = va_arg(args, size_t);
                    ft_puthex_stream(num, stream, uppercase, &count);
                }
                else
                {
                    unsigned int num = va_arg(args, unsigned int);
                    ft_puthex_stream(num, stream, uppercase, &count);
                }
            }
            else if (spec == 'p')
            {
                void *ptr = va_arg(args, void *);
                ft_putptr_stream(ptr, stream, &count);
            }
            else if (spec == 'b')
            {
                int b = va_arg(args, int);
                ft_putstr_stream(b ? "true" : "false", stream, &count);
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
            ft_putchar_stream(format[i], stream, &count);
        }
        i++;
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

