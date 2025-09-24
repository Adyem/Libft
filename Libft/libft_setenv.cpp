#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include <cstdlib>

int ft_setenv(const char *name, const char *value, int overwrite)
{
    int result;

    ft_errno = ER_SUCCESS;
    if (name == ft_nullptr || value == ft_nullptr || *name == '\0' || ft_strchr(name, '=') != ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (-1);
    }
    result = cmp_setenv(name, value, overwrite);
    if (result != 0)
        ft_errno = FT_ETERM;
    return (result);
}
