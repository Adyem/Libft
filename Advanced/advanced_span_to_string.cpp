#include "advanced.hpp"
#include "../CPP_class/class_string.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

ft_string adv_span_to_string(const char *buffer, size_t length)
{
    ft_string result;

    if (buffer == ft_nullptr && length != 0)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (ft_string(FT_ERR_INVALID_ARGUMENT));
    }
    if (length == 0)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (result);
    }
    result.assign(buffer, length);
    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(ft_string::last_operation_error());
        return (ft_string(ft_string::last_operation_error()));
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}
