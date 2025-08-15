#include "libft.hpp"
#include "../CPP_class/nullptr.hpp"
#include <cstdlib>

int ft_setenv(const char *name, const char *value, int overwrite)
{
    if (name == ft_nullptr || value == ft_nullptr)
        return (-1);
#if defined(_WIN32) || defined(_WIN64)
    if (!overwrite && std::getenv(name) != ft_nullptr)
        return (0);
    return (_putenv_s(name, value));
#else
    return (setenv(name, value, overwrite));
#endif
}

