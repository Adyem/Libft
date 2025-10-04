#ifndef PTHREAD_MUTEX_HPP
# define PTHREAD_MUTEX_HPP

#include "thread_id.hpp"
#include "../Template/atomic.hpp"

class pt_mutex
{
    private:
        ft_atomic<uint32_t>     _next;
        ft_atomic<uint32_t>     _serving;
        ft_atomic<pthread_t>    _owner;
        volatile bool           _lock;
        int                     _error;

        void    set_error(int error);

        pt_mutex(const pt_mutex&) = delete;
        pt_mutex& operator=(const pt_mutex&) = delete;
        pt_mutex(pt_mutex&&) = delete;
        pt_mutex& operator=(pt_mutex&&) = delete;

    public:
        pt_mutex();
        ~pt_mutex();

        const volatile bool &lockState() const;

        int     lock(pthread_t thread_id);
        int     unlock(pthread_t thread_id);
        int     try_lock(pthread_t thread_id);

        int     get_error() const;
        const char *get_error_str() const;
};

#endif

