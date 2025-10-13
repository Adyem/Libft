#ifndef PTHREAD_MUTEX_HPP
# define PTHREAD_MUTEX_HPP

#include <pthread.h>
#include <atomic>

class pt_mutex
{
    private:
        mutable std::atomic<pthread_t>    _owner;
        mutable bool                      _lock;
        mutable int                       _error;

        void    set_error(int error) const;

        pt_mutex(const pt_mutex&) = delete;
        pt_mutex& operator=(const pt_mutex&) = delete;
        pt_mutex(pt_mutex&&) = delete;
        pt_mutex& operator=(pt_mutex&&) = delete;

    public:
        pt_mutex();
        ~pt_mutex();

        const volatile bool &lockState() const;

        int     lock(pthread_t thread_id) const;
        int     unlock(pthread_t thread_id) const;
        int     try_lock(pthread_t thread_id) const;

        int     get_error() const;
        const char *get_error_str() const;
};

#endif
