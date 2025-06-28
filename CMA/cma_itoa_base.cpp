#include "CMA.hpp"
#include "../CPP_class/nullptr.hpp"

static int	calculate_length(int n, int base)
{
    int len = 0;
    unsigned int num = (n < 0) ? -n : n;
    if (num == 0)
        return (1);
    while (num)
    {
        num /= base;
        len++;
    }
    return (len);
}

char    *cma_itoa_base(int n, int base)
{
    const char digits[] = "0123456789ABCDEF";
    int negative = 0;
    int len;
    char *str;
    unsigned int num;

    if (base < 2 || base > 16)
        return (ft_nullptr);
    if (n < 0 && base == 10)
        negative = 1;
    num = (n < 0) ? -n : n;
    len = calculate_length(n, base);
    str = static_cast<char*>(cma_malloc(len + negative + 1));
    if (!str)
        return (ft_nullptr);
    str[len + negative] = '\0';
    while (len > 0)
    {
        str[len + negative - 1] = digits[num % base];
        num /= base;
        len--;
    }
    if (negative)
        str[0] = '-';
    return (str);
}
