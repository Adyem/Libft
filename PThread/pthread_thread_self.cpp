#include "pthread.hpp"
#include "../Errno/errno.hpp"

pt_thread_id_type pt_thread_self()
{
    pt_thread_id_type thread_id;

    thread_id = pt_thread_id;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (thread_id);
}
