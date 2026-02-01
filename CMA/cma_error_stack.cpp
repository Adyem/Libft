#include "cma_internal.hpp"
#include "../Errno/errno.hpp"

void cma_record_operation_error(int error_code)
{
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    return ;
}

void cma_record_internal_operation_error(int error_code)
{
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    return ;
}
