#ifndef MUTEX_HPP
# define MUTEX_HPP

#include "PThread.hpp"
#include <atomic>

class pt_mutex
{
private:
        alignas(64) std::atomic_flag _flag;
        volatile bool                _lock;
        int                         _error;

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
};

#endif
