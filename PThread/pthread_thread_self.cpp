#include "pthread.hpp"
#include "../Errno/errno.hpp"

pt_thread_id_type pt_thread_self()
{
    pt_thread_id_type thread_id;
    int error_code;

    thread_id = pt_thread_id;
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (thread_id);
}
