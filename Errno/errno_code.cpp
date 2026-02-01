#include "errno.hpp"
#include "errno_internal.hpp"

#include <atomic>

thread_local int ft_errno = FT_ERR_SUCCESSS;

unsigned long long ft_errno_next_operation_id(void)
{
    static std::atomic<unsigned long long> next_op_id(1);

    return (next_op_id.fetch_add(1, std::memory_order_relaxed));
}

static ft_error_stack &ft_global_error_stack()
{
    static thread_local ft_error_stack error_stack = {{}, 0, 0, 0, 0};

    return (error_stack);
}

static const ft_size_t ft_error_stack_capacity = 32;

static void ft_error_stack_shift_frames(ft_error_frame frames[], uint32_t &depth)
{
    if (depth < ft_error_stack_capacity)
        depth++;
    ft_size_t index = depth > 0 ? depth - 1 : 0;

    while (index > 0)
    {
        frames[index] = frames[index - 1];
        index--;
    }
}

unsigned long long ft_global_error_stack_push_entry(int error_code)
{
    ft_error_stack &error_stack = ft_global_error_stack();

    ft_error_stack_shift_frames(error_stack.frames, error_stack.depth);
    unsigned long long operation_id = ft_errno_next_operation_id();
    error_stack.frames[0].code = error_code;
    error_stack.frames[0].op_id = operation_id;
    error_stack.last_error = error_code;
    error_stack.last_op_id = operation_id;
    error_stack.next_op_id = operation_id + 1;
    return (operation_id);
}

void ft_global_error_stack_push(int error_code)
{
    ft_global_error_stack_push_entry(error_code);
    return ;
}

int ft_global_error_stack_pop_last(void)
{
    ft_error_stack &error_stack = ft_global_error_stack();

    if (error_stack.depth == 0)
        return (FT_ERR_SUCCESSS);
    int error_code = error_stack.frames[error_stack.depth - 1].code;

    error_stack.depth--;
    return (error_code);
}

int ft_global_error_stack_pop_newest(void)
{
    ft_error_stack &error_stack = ft_global_error_stack();

    if (error_stack.depth == 0)
        return (FT_ERR_SUCCESSS);
    int error_code = error_stack.frames[0].code;
    ft_size_t index = 0;

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
    ft_global_error_stack().depth = 0;
    return ;
}

int ft_global_error_stack_error_at(ft_size_t index)
{
    const ft_error_stack &error_stack = ft_global_error_stack();

    if (index == 0 || index > error_stack.depth)
        return (FT_ERR_SUCCESSS);
    return (error_stack.frames[index - 1].code);
}

int ft_global_error_stack_last_error(void)
{
    const ft_error_stack &error_stack = ft_global_error_stack();

    if (error_stack.depth == 0)
        return (FT_ERR_SUCCESSS);
    return (error_stack.frames[0].code);
}

ft_size_t ft_global_error_stack_depth(void)
{
    return (ft_global_error_stack().depth);
}

const char *ft_global_error_stack_error_str_at(ft_size_t index)
{
    int error_code = ft_global_error_stack_error_at(index);
    const char *error_string = ft_strerror(error_code);

    if (!error_string)
        error_string = "unknown error";
    return (error_string);
}

const char *ft_global_error_stack_last_error_str(void)
{
    int error_code = ft_global_error_stack_last_error();
    const char *error_string = ft_strerror(error_code);

    if (!error_string)
        error_string = "unknown error";
    return (error_string);
}
