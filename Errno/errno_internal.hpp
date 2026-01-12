#ifndef ERRNO_INTERNAL_HPP
# define ERRNO_INTERNAL_HPP

#include <mutex>
#include <cstdint>
#include "../Libft/libft.hpp"

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
    int32_t     code;
    uint32_t    op_id;
};

struct ft_error_stack
{
    ft_error_frame  frames[32];
    uint32_t        depth;
    uint32_t        next_op_id;
    int32_t         last_error;
    uint32_t        last_op_id;
};

struct ft_operation_error_stack
{
    int32_t     errors[20];
    uint32_t    count;
};

#include "../Libft/libft.hpp"

ft_errno_mutex_wrapper &ft_errno_mutex();
void ft_set_errno_locked(int error_code);
void ft_set_sys_errno_locked(int error_code);
void ft_global_error_stack_push(int error_code);
int ft_global_error_stack_pop_last(void);
int ft_global_error_stack_pop_newest(void);
void ft_global_error_stack_pop_all(void);
int ft_global_error_stack_error_at(ft_size_t index);
int ft_global_error_stack_last_error(void);
const char *ft_global_error_stack_error_str_at(ft_size_t index);
const char *ft_global_error_stack_last_error_str(void);

#endif
