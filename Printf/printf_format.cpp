#include "printf_internal.hpp"
#include "../Errno/errno.hpp"
#include <cstdarg>
#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <stddef.h>

int pf_printf_fd_v(int fd, const char *format, va_list args)
{
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
                break ;
            LengthModifier len_mod = LEN_NONE;
            int precision = 6;
            while (true)
            {
                if (format[index] == '.')
                {
                    index++;
                    precision = 0;
                    while (format[index] >= '0' && format[index] <= '9')
                    {
                        precision = precision * 10 + (format[index] - '0');
                        index++;
                    }
                }
                else if (format[index] == 'l')
                {
                    len_mod = LEN_L;
                    index++;
                }
                else if (format[index] == 'z')
                {
                    len_mod = LEN_Z;
                    index++;
                }
                else
                    break ;
            }
            char spec = format[index];
            if (spec == '\0')
                break ;
            if (spec == 'c')
            {
                char character = (char)va_arg(args, int);
                ft_putchar_fd(character, fd, &count);
            }
            else if (spec == 's')
            {
                char *string = va_arg(args, char *);
                ft_putstr_fd(string, fd, &count);
            }
            else if (spec == 'd' || spec == 'i')
            {
                if (len_mod == LEN_L)
                {
                    long number = va_arg(args, long);
                    ft_putnbr_fd(number, fd, &count);
                }
                else
                {
                    int number = va_arg(args, int);
                    ft_putnbr_fd(number, fd, &count);
                }
            }
            else if (spec == 'u')
            {
                if (len_mod == LEN_L)
                {
                    uintmax_t number = va_arg(args, unsigned long);
                    ft_putunsigned_fd(number, fd, &count);
                }
                else if (len_mod == LEN_Z)
                {
                    size_t number = va_arg(args, size_t);
                    ft_putunsigned_fd(number, fd, &count);
                }
                else
                {
                    unsigned int number = va_arg(args, unsigned int);
                    ft_putunsigned_fd(number, fd, &count);
                }
            }
            else if (spec == 'x' || spec == 'X')
            {
                bool uppercase = (spec == 'X');
                if (len_mod == LEN_L)
                {
                    uintmax_t number = va_arg(args, unsigned long);
                    ft_puthex_fd(number, fd, uppercase, &count);
                }
                else if (len_mod == LEN_Z)
                {
                    size_t number = va_arg(args, size_t);
                    ft_puthex_fd(number, fd, uppercase, &count);
                }
                else
                {
                    unsigned int number = va_arg(args, unsigned int);
                    ft_puthex_fd(number, fd, uppercase, &count);
                }
            }
            else if (spec == 'o')
            {
                if (len_mod == LEN_L)
                {
                    uintmax_t number = va_arg(args, unsigned long);
                    ft_putoctal_fd(number, fd, &count);
                }
                else if (len_mod == LEN_Z)
                {
                    size_t number = va_arg(args, size_t);
                    ft_putoctal_fd(number, fd, &count);
                }
                else
                {
                    unsigned int number = va_arg(args, unsigned int);
                    ft_putoctal_fd(number, fd, &count);
                }
            }
            else if (spec == 'f')
            {
                double number = va_arg(args, double);
                ft_putfloat_fd(number, fd, &count, precision);
            }
            else if (spec == 'e' || spec == 'E')
            {
                bool uppercase = (spec == 'E');
                double number = va_arg(args, double);
                ft_putscientific_fd(number, uppercase, fd, &count, precision);
            }
            else if (spec == 'g' || spec == 'G')
            {
                bool uppercase = (spec == 'G');
                double number = va_arg(args, double);
                ft_putgeneral_fd(number, uppercase, fd, &count, precision);
            }
            else if (spec == 'p')
            {
                void *pointer = va_arg(args, void *);
                ft_putptr_fd(pointer, fd, &count);
            }
            else if (spec == 'b')
            {
                int boolean_value = va_arg(args, int);
                if (boolean_value)
                    ft_putstr_fd("true", fd, &count);
                else
                    ft_putstr_fd("false", fd, &count);
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
                ft_putchar_fd('%', fd, &count);
            else
            {
                ft_putchar_fd('%', fd, &count);
                ft_putchar_fd(spec, fd, &count);
            }
        }
        else
            ft_putchar_fd(format[index], fd, &count);
        index++;
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
