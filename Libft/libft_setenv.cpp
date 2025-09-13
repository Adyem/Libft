#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include <cstdlib>

int ft_setenv(const char *name, const char *value, int overwrite)
{
    if (name == ft_nullptr || value == ft_nullptr || *name == '\0' || ft_strchr(name, '=') != ft_nullptr)
        return (-1);
    return (cmp_setenv(name, value, overwrite));
}
