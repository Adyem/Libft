#include "libft.hpp"
#include "limits.hpp"
#include "../Errno/errno.hpp"

int ft_strlen(const char *string)
{
    size_t len;

    ft_errno = ER_SUCCESS;
    len = ft_strlen_size_t(string);
    if (len > static_cast<size_t>(FT_INT_MAX))
    {
        ft_errno = FT_ERANGE;
        return (FT_INT_MAX);
    }
    return (static_cast<int>(len));
}
