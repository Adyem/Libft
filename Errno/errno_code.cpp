#include "errno.hpp"
#include "errno_internal.hpp"

#include <cstdlib>

ft_errno_mutex_wrapper::ft_errno_mutex_wrapper(void)
{
    return ;
}

ft_errno_mutex_wrapper::~ft_errno_mutex_wrapper(void)
{
    return ;
}

void ft_errno_mutex_wrapper::lock(void)
{
    try
    {
        this->_mutex.lock();
    }
    catch (...)
    {
        std::abort();
    }
    return ;
}

void ft_errno_mutex_wrapper::unlock(void)
{
    try
    {
        this->_mutex.unlock();
    }
    catch (...)
    {
        std::abort();
    }
    return ;
}

ft_errno_mutex_wrapper &ft_errno_mutex()
{
    static ft_errno_mutex_wrapper errno_mutex;

    return (errno_mutex);
}

static ft_operation_error_stack &ft_global_error_stack()
{
    static thread_local ft_operation_error_stack error_stack = {{}, 0};

    return (error_stack);
}

void ft_set_errno_locked(int error_code)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());

    ft_errno = error_code;
    return ;
}

void ft_set_sys_errno_locked(int error_code)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());

    ft_sys_errno = error_code;
    return ;
}

void ft_global_error_stack_push(int error_code)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_operation_error_stack &error_stack = ft_global_error_stack();
    ft_size_t index;

    if (error_stack.count < 20)
        error_stack.count++;
    index = error_stack.count;
    while (index > 0)
    {
        error_stack.errors[index] = error_stack.errors[index - 1];
        index--;
    }
    error_stack.errors[0] = error_code;
    return ;
}

int ft_global_error_stack_pop_last(void)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_operation_error_stack &error_stack = ft_global_error_stack();
    int error_code;

    if (error_stack.count == 0)
        return (FT_ERR_SUCCESSS);
    error_code = error_stack.errors[error_stack.count - 1];
    error_stack.count--;
    return (error_code);
}

int ft_global_error_stack_pop_newest(void)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_operation_error_stack &error_stack = ft_global_error_stack();
    ft_size_t index;
    int error_code;

    if (error_stack.count == 0)
        return (FT_ERR_SUCCESSS);
    error_code = error_stack.errors[0];
    index = 0;
    while (index + 1 < error_stack.count)
    {
        error_stack.errors[index] = error_stack.errors[index + 1];
        index++;
    }
    error_stack.count--;
    return (error_code);
}

void ft_global_error_stack_pop_all(void)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_operation_error_stack &error_stack = ft_global_error_stack();

    error_stack.count = 0;
    return ;
}

int ft_global_error_stack_error_at(ft_size_t index)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_operation_error_stack &error_stack = ft_global_error_stack();

    if (index == 0 || index > error_stack.count)
        return (FT_ERR_SUCCESSS);
    return (error_stack.errors[index - 1]);
}

int ft_global_error_stack_last_error(void)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_operation_error_stack &error_stack = ft_global_error_stack();

    if (error_stack.count == 0)
        return (FT_ERR_SUCCESSS);
    return (error_stack.errors[0]);
}

const char *ft_global_error_stack_error_str_at(ft_size_t index)
{
    int error_code;
    const char *error_string;

    error_code = ft_global_error_stack_error_at(index);
    error_string = ft_strerror(error_code);
    if (!error_string)
        error_string = "unknown error";
    return (error_string);
}

const char *ft_global_error_stack_last_error_str(void)
{
    int error_code;
    const char *error_string;

    error_code = ft_global_error_stack_last_error();
    error_string = ft_strerror(error_code);
    if (!error_string)
        error_string = "unknown error";
    return (error_string);
}

int &ft_errno_reference(void)
{
    static thread_local int thread_errno = FT_ERR_SUCCESSS;

    return (thread_errno);
}

int &ft_sys_errno_reference(void)
{
    static thread_local int thread_sys_errno = FT_SYS_ERR_SUCCESS;

    return (thread_sys_errno);
}
