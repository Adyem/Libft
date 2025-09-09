#include "libft.hpp"
#include "limits.hpp"

int ft_strlen(const char *string)
{
    size_t len = ft_strlen_size_t(string);
    if (len > static_cast<size_t>(FT_INT_MAX))
        return (FT_INT_MAX);
    return (static_cast<int>(len));
}
