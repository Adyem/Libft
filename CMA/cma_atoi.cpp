#include "CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

int *cma_atoi(const char *string)
{
    int *number;
    int error_code;
    int validate_result;

    validate_result = ft_validate_int(string);
    error_code = ft_global_error_stack_drop_last_error();
    if (validate_result != FT_SUCCESS)
    {
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    number = static_cast<int *>(cma_malloc(sizeof(int)));
    error_code = ft_global_error_stack_drop_last_error();
    if (!number)
    {
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    *number = ft_atoi(string);
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (number);
}
