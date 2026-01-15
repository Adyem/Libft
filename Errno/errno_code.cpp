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

static ft_error_stack &ft_global_error_stack()
{
    static thread_local ft_error_stack error_stack = {{}, 0, 0, 0, 0};

    return (error_stack);
}

static void ft_global_error_stack_push_unlocked(ft_error_stack &error_stack, int error_code)
{
    ft_size_t index;

    if (error_stack.depth < 32)
        error_stack.depth++;
    index = error_stack.depth;
    while (index > 0)
    {
        error_stack.frames[index] = error_stack.frames[index - 1];
        index--;
    }
    error_stack.frames[0].code = error_code;
    error_stack.frames[0].op_id = error_stack.next_op_id++;
    error_stack.last_error = error_code;
    error_stack.last_op_id = error_stack.frames[0].op_id;
    return ;
}

void ft_set_errno_locked(int error_code)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_error_stack &error_stack = ft_global_error_stack();

    ft_global_error_stack_push_unlocked(error_stack, error_code);
    return ;
}

void ft_set_sys_errno_locked(int error_code)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_error_stack &error_stack = ft_global_error_stack();

    ft_global_error_stack_push_unlocked(error_stack, error_code);
    return ;
}

void ft_global_error_stack_push(int error_code)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_error_stack &error_stack = ft_global_error_stack();

    ft_global_error_stack_push_unlocked(error_stack, error_code);
    return ;
}

int ft_global_error_stack_pop_last(void)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_error_stack &error_stack = ft_global_error_stack();
    int error_code;

    if (error_stack.depth == 0)
        return (FT_ERR_SUCCESSS);
    error_code = error_stack.frames[error_stack.depth - 1].code;
    error_stack.depth--;
    return (error_code);
}

int ft_global_error_stack_pop_newest(void)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_error_stack &error_stack = ft_global_error_stack();
    ft_size_t index;
    int error_code;

    if (error_stack.depth == 0)
        return (FT_ERR_SUCCESSS);
    error_code = error_stack.frames[0].code;
    index = 0;
    while (index + 1 < error_stack.depth)
    {
        error_stack.frames[index] = error_stack.frames[index + 1];
        index++;
    }
    error_stack.depth--;
    return (error_code);
}

void ft_global_error_stack_pop_all(void)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_error_stack &error_stack = ft_global_error_stack();

    error_stack.depth = 0;
    return ;
}

int ft_global_error_stack_error_at(ft_size_t index)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_error_stack &error_stack = ft_global_error_stack();

    if (index == 0 || index > error_stack.depth)
        return (FT_ERR_SUCCESSS);
    return (error_stack.frames[index - 1].code);
}

int ft_global_error_stack_last_error(void)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_error_stack &error_stack = ft_global_error_stack();

    if (error_stack.depth == 0)
        return (FT_ERR_SUCCESSS);
    return (error_stack.frames[0].code);
}

unsigned long long ft_global_error_stack_get_id_at(ft_size_t index)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_error_stack &error_stack = ft_global_error_stack();

    if (index == 0 || index > error_stack.depth)
        return (0);
    return (error_stack.frames[index - 1].op_id);
}

ft_size_t ft_global_error_stack_find_by_id(unsigned long long id)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_error_stack &error_stack = ft_global_error_stack();

    for (ft_size_t i = 0; i < error_stack.depth; i++)
    {
        if (error_stack.frames[i].op_id == id)
            return (i + 1);
    }
    return (0);
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
