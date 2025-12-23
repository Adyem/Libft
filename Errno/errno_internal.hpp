#ifndef ERRNO_INTERNAL_HPP
# define ERRNO_INTERNAL_HPP

#include <mutex>

class ft_errno_mutex_wrapper
{
    private:
        std::mutex _mutex;

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

ft_errno_mutex_wrapper &ft_errno_mutex();
void ft_set_errno_locked(int error_code);
void ft_set_sys_errno_locked(int error_code);

#endif
