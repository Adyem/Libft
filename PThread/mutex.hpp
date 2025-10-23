#ifndef PTHREAD_MUTEX_HPP
# define PTHREAD_MUTEX_HPP

#include <pthread.h>
#include <atomic>
#include <time.h>

class pt_mutex
{
    private:
        mutable std::atomic<pthread_t>    _owner;
        mutable std::atomic<bool>         _lock;
        mutable int                       _error;
        mutable pthread_mutex_t           _native_mutex;
        mutable bool                      _native_initialized;

        void    set_error(int error) const;
        bool    ensure_native_mutex() const;

        pt_mutex(const pt_mutex&) = delete;
        pt_mutex& operator=(const pt_mutex&) = delete;
        pt_mutex(pt_mutex&&) = delete;
        pt_mutex& operator=(pt_mutex&&) = delete;

    public:
        pt_mutex();
        ~pt_mutex();

        bool    lockState() const;

        int     lock(pthread_t thread_id) const;
        int     unlock(pthread_t thread_id) const;
        int     try_lock(pthread_t thread_id) const;
        int     try_lock_until(pthread_t thread_id, const struct timespec &absolute_time) const;
        int     try_lock_for(pthread_t thread_id, const struct timespec &relative_time) const;

        int     get_error() const;
        const char *get_error_str() const;
};

#endif
