#ifndef RECURSIVE_MUTEX_HPP
# define RECURSIVE_MUTEX_HPP

#include <pthread.h>
#include <atomic>
#include <time.h>
#include <type_traits>
#include <cstddef>

#include "mutex.hpp"

class pt_recursive_mutex
{
    private:
        mutable std::atomic<pthread_t>    _owner;
        mutable std::atomic<bool>         _lock;
        mutable std::atomic<std::size_t>  _lock_depth;
        mutable int                       _error;
        mutable pthread_mutex_t           _native_mutex;
        mutable bool                      _native_initialized;
        mutable pt_mutex                  *_state_mutex;

        void    set_error(int error) const;
        bool    ensure_native_mutex() const;
        int     lock_internal(bool *lock_acquired) const;
        void    unlock_internal(bool lock_acquired) const;
        void    teardown_thread_safety();

        pt_recursive_mutex(const pt_recursive_mutex&) = delete;
        pt_recursive_mutex& operator=(const pt_recursive_mutex&) = delete;
        pt_recursive_mutex(pt_recursive_mutex&&) = delete;
        pt_recursive_mutex& operator=(pt_recursive_mutex&&) = delete;

    public:
        pt_recursive_mutex();
        ~pt_recursive_mutex();

        bool    lockState() const;

        int     lock_state(bool *lock_acquired) const;
        void    unlock_state(bool lock_acquired) const;

        int     lock(pthread_t thread_id) const;
        int     unlock(pthread_t thread_id) const;
        int     try_lock(pthread_t thread_id) const;
        int     try_lock_until(pthread_t thread_id, const struct timespec &absolute_time) const;
        int     try_lock_for(pthread_t thread_id, const struct timespec &relative_time) const;

        bool    is_owned_by_thread(pthread_t thread_id) const;

        pthread_mutex_t   *get_native_mutex() const;

        int     get_error() const;
        const char *get_error_str() const;
};

static_assert(!std::is_copy_constructible<pt_recursive_mutex>::value, "pt_recursive_mutex cannot be copied");
static_assert(!std::is_copy_assignable<pt_recursive_mutex>::value, "pt_recursive_mutex cannot be copy assigned");

#endif
