#ifndef ERRNO_INTERNAL_HPP
# define ERRNO_INTERNAL_HPP

#include <mutex>
#include <cstdint>
#include "errno.hpp"

class ft_errno_mutex_wrapper
{
    private:
        std::recursive_mutex _mutex;

    public:
        ft_errno_mutex_wrapper(void);
        ~ft_errno_mutex_wrapper(void);
        ft_errno_mutex_wrapper(const ft_errno_mutex_wrapper &other) = delete;
        ft_errno_mutex_wrapper &operator=(const ft_errno_mutex_wrapper &other) = delete;
        ft_errno_mutex_wrapper(ft_errno_mutex_wrapper &&other) = delete;
        ft_errno_mutex_wrapper &operator=(ft_errno_mutex_wrapper &&other) = delete;

        void lock(void);
        void unlock(void);
};

struct ft_error_frame
{
    int32_t             code;
    unsigned long long  op_id;
};

struct ft_error_stack
{
    ft_error_frame  frames[32];
    uint32_t        depth;
    unsigned long long        next_op_id;
    int32_t         last_error;
    unsigned long long        last_op_id;
};

struct ft_operation_error_stack
{
    int32_t     errors[20];
    unsigned long long    op_ids[20];
    uint32_t    count;
};

void ft_error_stack_push_entry_with_id(ft_error_stack *error_stack,
        int error_code, unsigned long long op_id);
int ft_error_stack_pop_last(ft_error_stack *error_stack);
int ft_error_stack_pop_newest(ft_error_stack *error_stack);
void ft_error_stack_pop_all(ft_error_stack *error_stack);
int ft_error_stack_error_at(const ft_error_stack *error_stack,
        ft_size_t index);
int ft_error_stack_last_error(const ft_error_stack *error_stack);
ft_size_t ft_error_stack_depth(const ft_error_stack *error_stack);
unsigned long long ft_error_stack_get_id_at(const ft_error_stack *error_stack,
        ft_size_t index);
ft_size_t ft_error_stack_find_by_id(const ft_error_stack *error_stack,
        unsigned long long id);
const char *ft_error_stack_error_str_at(const ft_error_stack *error_stack,
        ft_size_t index);
const char *ft_error_stack_last_error_str(const ft_error_stack *error_stack);

void ft_operation_error_stack_push(ft_operation_error_stack *error_stack,
        int error_code, unsigned long long op_id);
int ft_operation_error_stack_pop_last(ft_operation_error_stack *error_stack);
int ft_operation_error_stack_pop_newest(ft_operation_error_stack *error_stack);
void ft_operation_error_stack_pop_all(ft_operation_error_stack *error_stack);
int ft_operation_error_stack_error_at(const ft_operation_error_stack *error_stack,
        ft_size_t index);
int ft_operation_error_stack_last_error(const ft_operation_error_stack *error_stack);
unsigned long long ft_operation_error_stack_last_id(
        const ft_operation_error_stack *error_stack);

ft_errno_mutex_wrapper &ft_errno_mutex();
void ft_set_errno_locked(int error_code);
void ft_set_sys_errno_locked(int error_code);

#endif
