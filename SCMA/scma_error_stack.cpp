#include "scma_internal.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"

thread_local ft_operation_error_stack g_scma_operation_errors = {{}, {}, 0};

unsigned long long scma_record_operation_error(int error_code)
{
    unsigned long long operation_id;

    operation_id = ft_global_error_stack_push_entry(error_code);
    ft_operation_error_stack_push(&g_scma_operation_errors, error_code, operation_id);
    return (operation_id);
}

int scma_pop_operation_error(void)
{
    int error_code;

    error_code = ft_global_error_stack_pop_newest();
    ft_operation_error_stack_pop_newest(&g_scma_operation_errors);
    return (error_code);
}
