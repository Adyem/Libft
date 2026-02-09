#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

int *adv_atoi(const char *string)
{
    int *number;
    int validate_result;

    validate_result = ft_validate_int(string);
    if (validate_result != FT_SUCCESS)
        return (ft_nullptr);
    (void)ft_global_error_stack_drop_last_error();
    number = static_cast<int *>(cma_malloc(sizeof(int)));
    if (!number)
        return (ft_nullptr);
    (void)ft_global_error_stack_drop_last_error();
    *number = ft_atoi(string, ft_nullptr);
    if (ft_global_error_stack_peek_last_error())
        return (cma_free(number), ft_nullptr);
    return (number);
}
