#include "cma_internal.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"

thread_local ft_operation_error_stack g_cma_operation_errors = {{}, {}, 0};

void cma_record_operation_error(int error_code)
{
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    ft_operation_error_stack_push(&g_cma_operation_errors, error_code, operation_id);
    return ;
}

void cma_record_internal_operation_error(int error_code)
{
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    ft_operation_error_stack_push(&g_cma_operation_errors, error_code, operation_id);
    return ;
}
