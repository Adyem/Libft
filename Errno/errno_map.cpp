#include "errno.hpp"
#include "../Compatebility/compatebility_internal.hpp"

int ft_map_system_error(int error_code)
{
    int mapped_error;

    mapped_error = cmp_map_system_error_to_ft(error_code);
    ft_sys_errno = mapped_error;
    return (mapped_error);
}

int ft_set_errno_from_system_error(int error_code)
{
    int mapped_error;

    mapped_error = ft_map_system_error(error_code);
    ft_errno = mapped_error;
    return (mapped_error);
}
