#include "system_utils.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "environment_lock.hpp"
#include <cstdlib>

char *ft_getenv(const char *name)
{
    char    *value;

    if (name == ft_nullptr || *name == '\0')
        return (ft_nullptr);
    if (ft_environment_lock() != 0)
        return (ft_nullptr);
    value = getenv(name);
    if (ft_environment_unlock() != 0)
        return (ft_nullptr);
    return (value);
}
