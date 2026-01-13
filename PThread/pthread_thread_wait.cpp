#include "pthread.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include <atomic>
#include <errno.h>

int pt_thread_wait_uint32(std::atomic<uint32_t> *address, uint32_t expected_value)
{
    int return_value;
    int error_code;

    return_value = cmp_thread_wait_uint32(address, expected_value);
    if (return_value != 0)
    {
        error_code = ft_map_system_error(errno);
        ft_global_error_stack_push(error_code);
        return (return_value);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (return_value);
}

int pt_thread_wake_one_uint32(std::atomic<uint32_t> *address)
{
    int return_value;
    int error_code;

    return_value = cmp_thread_wake_one_uint32(address);
    if (return_value != 0)
    {
        error_code = ft_map_system_error(errno);
        ft_global_error_stack_push(error_code);
        return (return_value);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (return_value);
}
