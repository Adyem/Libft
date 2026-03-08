#include "system_utils.hpp"
#include "../Basic/basic.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "System_utils_environment_lock.hpp"

int32_t ft_setenv(const char *name, const char *value, int32_t overwrite)
{
    char *invalid_character;
    int32_t result;

    if (name == ft_nullptr || value == ft_nullptr || *name == '\0')
        return (-1);
    invalid_character = ft_strchr(name, '=');
    if (invalid_character != ft_nullptr)
        return (-1);
    if (ft_environment_lock() != 0)
        return (-1);
    result = cmp_setenv(name, value, overwrite);
    if (ft_environment_unlock() != 0)
        return (-1);
    return (result);
}
