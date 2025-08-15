#include "libft.hpp"

static int ft_get_digit(char c)
{
    if (c >= '0' && c <= '9')
        return (c - '0');
    if (c >= 'a' && c <= 'z')
        return (c - 'a' + 10);
    if (c >= 'A' && c <= 'Z')
        return (c - 'A' + 10);
    return (-1);
}

long ft_strtol(const char *nptr, char **endptr, int base)
{
    const char *s = nptr;
    long sign = 1;
    unsigned long result = 0;
    int digit;

    while (*s == ' ' || (*s >= '\t' && *s <= '\r'))
        s++;
    if (*s == '+' || *s == '-')
    {
        if (*s == '-')
            sign = -1;
        s++;
    }
    if (base == 0)
    {
        if (*s == '0')
        {
            if (s[1] == 'x' || s[1] == 'X')
            {
                base = 16;
                s += 2;
            }
            else
                base = 8;
        }
        else
            base = 10;
    }
    else if (base == 16 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
        s += 2;
    while ((digit = ft_get_digit(*s)) >= 0 && digit < base)
    {
        result = result * base + digit;
        s++;
    }
    if (endptr)
        *endptr = (char *)s;
    return ((long)(result * sign));
}
