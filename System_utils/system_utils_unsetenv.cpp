#include "system_utils.hpp"
#include "../Basic/basic.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "environment_lock.hpp"

int ft_unsetenv(const char *name)
{
    char *invalid_character;
    int result;

    if (name == ft_nullptr || *name == '\0')
        return (-1);
    invalid_character = ft_strchr(name, '=');
    if (invalid_character != ft_nullptr)
        return (-1);
    if (ft_environment_lock() != 0)
        return (-1);
    result = cmp_unsetenv(name);
    if (ft_environment_unlock() != 0)
        return (-1);
    return (result);
}
