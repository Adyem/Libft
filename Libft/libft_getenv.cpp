#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <cstdlib>

char    *ft_getenv(const char *name)
{
    char    *value;

    ft_errno = ER_SUCCESS;
    if (name == ft_nullptr || *name == '\0')
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    value = std::getenv(name);
    return (value);
}

