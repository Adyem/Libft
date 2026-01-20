#include "errno.hpp"
#include "errno_internal.hpp"

#include <atomic>
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

static void ft_error_stack_shift_frames(ft_error_frame frames[], uint32_t &depth,
        ft_size_t capacity)
{
    if (depth < capacity)
        depth++;
    ft_size_t index = depth > 0 ? depth - 1 : 0;

    while (index > 0)
    {
        frames[index] = frames[index - 1];
        index--;
    }
}

void ft_error_stack_push_entry_with_id_unlocked(ft_error_stack *error_stack,
        int error_code, unsigned long long op_id)
{
    ft_error_stack_shift_frames(error_stack->frames, error_stack->depth, 32);
    error_stack->frames[0].code = error_code;
    error_stack->frames[0].op_id = op_id;
    error_stack->last_error = error_code;
    error_stack->last_op_id = op_id;
    error_stack->next_op_id = op_id + 1;
    return ;
}

int ft_error_stack_pop_last_unlocked(ft_error_stack *error_stack)
{
    if (error_stack->depth == 0)
        return (FT_ERR_SUCCESSS);
    int error_code = error_stack->frames[error_stack->depth - 1].code;

    error_stack->depth--;
    return (error_code);
}

int ft_error_stack_pop_newest_unlocked(ft_error_stack *error_stack)
{
    if (error_stack->depth == 0)
        return (FT_ERR_SUCCESSS);
    int error_code = error_stack->frames[0].code;
    ft_size_t index = 0;

    while (index + 1 < error_stack->depth)
    {
        error_stack->frames[index] = error_stack->frames[index + 1];
        index++;
    }
    error_stack->depth--;
    return (error_code);
}

void ft_error_stack_pop_all_unlocked(ft_error_stack *error_stack)
{
    error_stack->depth = 0;
    return ;
}

int ft_error_stack_error_at_unlocked(const ft_error_stack *error_stack,
        ft_size_t index)
{
    if (index == 0 || index > error_stack->depth)
        return (FT_ERR_SUCCESSS);
    return (error_stack->frames[index - 1].code);
}

int ft_error_stack_last_error_unlocked(const ft_error_stack *error_stack)
{
    if (error_stack->depth == 0)
        return (FT_ERR_SUCCESSS);
    return (error_stack->frames[0].code);
}

ft_size_t ft_error_stack_depth_unlocked(const ft_error_stack *error_stack)
{
    return (error_stack->depth);
}

unsigned long long ft_error_stack_get_id_at_unlocked(const ft_error_stack *error_stack,
        ft_size_t index)
{
    if (index == 0 || index > error_stack->depth)
        return (0);
    return (error_stack->frames[index - 1].op_id);
}

ft_size_t ft_error_stack_find_by_id_unlocked(const ft_error_stack *error_stack,
        unsigned long long id)
{
    for (ft_size_t i = 0; i < error_stack->depth; i++)
    {
        if (error_stack->frames[i].op_id == id)
            return (i + 1);
    }
    return (0);
}

const char *ft_error_stack_error_str_at_unlocked(const ft_error_stack *error_stack,
        ft_size_t index)
{
    int error_code = ft_error_stack_error_at_unlocked(error_stack, index);
    const char *error_string = ft_strerror(error_code);

    if (!error_string)
        error_string = "unknown error";
    return (error_string);
}

const char *ft_error_stack_last_error_str_unlocked(const ft_error_stack *error_stack)
{
    int error_code = ft_error_stack_last_error_unlocked(error_stack);
    const char *error_string = ft_strerror(error_code);

    if (!error_string)
        error_string = "unknown error";
    return (error_string);
}

static void ft_operation_error_stack_shift_entries(ft_operation_error_stack *error_stack)
{
    if (error_stack->count < 20)
        error_stack->count++;
    ft_size_t index = error_stack->count > 0 ? error_stack->count - 1 : 0;

    while (index > 0)
    {
        error_stack->errors[index] = error_stack->errors[index - 1];
        error_stack->op_ids[index] = error_stack->op_ids[index - 1];
        index--;
    }
}

void ft_operation_error_stack_push_unlocked(ft_operation_error_stack *error_stack,
        int error_code, unsigned long long op_id)
{
    ft_operation_error_stack_shift_entries(error_stack);
    error_stack->errors[0] = error_code;
    error_stack->op_ids[0] = op_id;
    return ;
}

int ft_operation_error_stack_pop_last_unlocked(ft_operation_error_stack *error_stack)
{
    if (error_stack->count == 0)
        return (FT_ERR_SUCCESSS);
    error_stack->count--;
    return (error_stack->errors[error_stack->count]);
}

int ft_operation_error_stack_pop_newest_unlocked(ft_operation_error_stack *error_stack)
{
    if (error_stack->count == 0)
        return (FT_ERR_SUCCESSS);
    int popped_error = error_stack->errors[0];
    ft_size_t index = 0;

    while (index + 1 < error_stack->count)
    {
        error_stack->errors[index] = error_stack->errors[index + 1];
        error_stack->op_ids[index] = error_stack->op_ids[index + 1];
        index++;
    }
    error_stack->count--;
    return (popped_error);
}

void ft_operation_error_stack_pop_all_unlocked(ft_operation_error_stack *error_stack)
{
    error_stack->count = 0;
    return ;
}

int ft_operation_error_stack_error_at_unlocked(const ft_operation_error_stack *error_stack,
        ft_size_t index)
{
    if (index == 0 || index > error_stack->count)
        return (FT_ERR_SUCCESSS);
    return (error_stack->errors[index - 1]);
}

int ft_operation_error_stack_last_error_unlocked(const ft_operation_error_stack *error_stack)
{
    if (error_stack->count == 0)
        return (FT_ERR_SUCCESSS);
    return (error_stack->errors[0]);
}

unsigned long long ft_operation_error_stack_last_id_unlocked(
        const ft_operation_error_stack *error_stack)
{
    if (error_stack->count == 0)
        return (0);
    return (error_stack->op_ids[0]);
}


void ft_set_errno_locked(int error_code)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_error_stack &error_stack = ft_global_error_stack();
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    ft_error_stack_push_entry_with_id_unlocked(&error_stack, error_code, operation_id);
    return ;
}

void ft_set_sys_errno_locked(int error_code)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_error_stack &error_stack = ft_global_error_stack();
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    ft_error_stack_push_entry_with_id_unlocked(&error_stack, error_code, operation_id);
    return ;
}

unsigned long long ft_global_error_stack_push_entry_with_id(int error_code, unsigned long long op_id)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_error_stack &error_stack = ft_global_error_stack();

    ft_error_stack_push_entry_with_id_unlocked(&error_stack, error_code, op_id);
    return (op_id);
}

unsigned long long ft_global_error_stack_push_entry(int error_code)
{
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    return (operation_id);
}

void ft_global_error_stack_push(int error_code)
{
    ft_global_error_stack_push_entry(error_code);
    return ;
}

int ft_global_error_stack_pop_last(void)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_error_stack &error_stack = ft_global_error_stack();
    return (ft_error_stack_pop_last_unlocked(&error_stack));
}

int ft_global_error_stack_pop_newest(void)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_error_stack &error_stack = ft_global_error_stack();
    return (ft_error_stack_pop_newest_unlocked(&error_stack));
}

void ft_global_error_stack_pop_all(void)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_error_stack &error_stack = ft_global_error_stack();

    ft_error_stack_pop_all_unlocked(&error_stack);
    return ;
}

int ft_global_error_stack_error_at(ft_size_t index)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_error_stack &error_stack = ft_global_error_stack();

    return (ft_error_stack_error_at_unlocked(&error_stack, index));
}

int ft_global_error_stack_last_error(void)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_error_stack &error_stack = ft_global_error_stack();

    return (ft_error_stack_last_error_unlocked(&error_stack));
}

ft_size_t ft_global_error_stack_depth(void)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_error_stack &error_stack = ft_global_error_stack();

    return (ft_error_stack_depth_unlocked(&error_stack));
}

unsigned long long ft_global_error_stack_get_id_at(ft_size_t index)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_error_stack &error_stack = ft_global_error_stack();

    return (ft_error_stack_get_id_at_unlocked(&error_stack, index));
}

ft_size_t ft_global_error_stack_find_by_id(unsigned long long id)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_error_stack &error_stack = ft_global_error_stack();

    return (ft_error_stack_find_by_id_unlocked(&error_stack, id));
}

const char *ft_global_error_stack_error_str_at(ft_size_t index)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_error_stack &error_stack = ft_global_error_stack();

    return (ft_error_stack_error_str_at_unlocked(&error_stack, index));
}

const char *ft_global_error_stack_last_error_str(void)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());
    ft_error_stack &error_stack = ft_global_error_stack();

    return (ft_error_stack_last_error_str_unlocked(&error_stack));
}

void ft_operation_error_stack_push(ft_operation_error_stack &error_stack, int error_code, unsigned long long op_id)
{
    ft_operation_error_stack_push_unlocked(&error_stack, error_code, op_id);
    return ;
}

int ft_operation_error_stack_pop_last(ft_operation_error_stack &error_stack)
{
    return (ft_operation_error_stack_pop_last_unlocked(&error_stack));
}

int ft_operation_error_stack_pop_newest(ft_operation_error_stack &error_stack)
{
    return (ft_operation_error_stack_pop_newest_unlocked(&error_stack));
}

void ft_operation_error_stack_pop_all(ft_operation_error_stack &error_stack)
{
    ft_operation_error_stack_pop_all_unlocked(&error_stack);
    return ;
}

int ft_operation_error_stack_error_at(const ft_operation_error_stack &error_stack, ft_size_t index)
{
    return (ft_operation_error_stack_error_at_unlocked(&error_stack, index));
}

int ft_operation_error_stack_last_error(const ft_operation_error_stack &error_stack)
{
    return (ft_operation_error_stack_last_error_unlocked(&error_stack));
}

unsigned long long ft_operation_error_stack_last_id(const ft_operation_error_stack &error_stack)
{
    return (ft_operation_error_stack_last_id_unlocked(&error_stack));
}
