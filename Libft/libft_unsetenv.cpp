#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <cstdlib>

int ft_unsetenv(const char *name)
{
    if (name == ft_nullptr)
        return (-1);
#if defined(_WIN32) || defined(_WIN64)
    return (_putenv_s(name, ""));
#else
    return (unsetenv(name));
#endif
}

