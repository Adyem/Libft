#include "libft.hpp"

unsigned long ft_strtoul(const char *nptr, char **endptr, int base)
{
    long value = ft_strtol(nptr, endptr, base);
    return (static_cast<unsigned long>(value));
}
