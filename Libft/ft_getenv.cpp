#include "libft.hpp"
#include "../CPP_class/nullptr.hpp"
#include <cstdlib>

char    *ft_getenv(const char *name)
{
    if (name == ft_nullptr || *name == '\0')
        return (ft_nullptr);
    return (std::getenv(name));
}

