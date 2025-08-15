#include "libft.hpp"

unsigned long ft_strtoul(const char *nptr, char **endptr, int base)
{
    long value = ft_strtol(nptr, endptr, base);
    return ((unsigned long)value);
}
