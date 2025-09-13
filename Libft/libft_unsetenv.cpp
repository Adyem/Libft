#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include <cstdlib>

int ft_unsetenv(const char *name)
{
    if (name == ft_nullptr)
        return (-1);
    return (cmp_unsetenv(name));
}
