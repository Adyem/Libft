#include "scma_internal.hpp"
#include "../Errno/errno.hpp"

unsigned long long scma_record_operation_error(int error_code)
{
    return (ft_global_error_stack_push_entry(error_code));
}

int scma_pop_operation_error(void)
{
    return (ft_global_error_stack_pop_newest());
}
