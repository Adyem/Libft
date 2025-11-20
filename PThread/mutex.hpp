#ifndef PTHREAD_MUTEX_HPP
# define PTHREAD_MUTEX_HPP

#include <pthread.h>
#include <atomic>
#include <time.h>
#include <type_traits>

class pt_mutex
{
    private:
        mutable std::atomic<pthread_t>    _owner;
        mutable std::atomic<bool>         _lock;
        mutable int                       _error;
        mutable pthread_mutex_t           _native_mutex;
        mutable bool                      _native_initialized;
        mutable pt_mutex                  *_state_mutex;
        bool                              _thread_safe_enabled;

        void    set_error(int error) const;
        bool    ensure_native_mutex() const;
        int     lock_internal(bool *lock_acquired) const;
        void    unlock_internal(bool lock_acquired) const;
        void    teardown_thread_safety();

        pt_mutex(const pt_mutex&) = delete;
        pt_mutex& operator=(const pt_mutex&) = delete;
        pt_mutex(pt_mutex&&) = delete;
        pt_mutex& operator=(pt_mutex&&) = delete;

    public:
        pt_mutex();
        ~pt_mutex();

        bool    lockState() const;

        int     enable_thread_safety();
        void    disable_thread_safety();
        bool    is_thread_safe() const;
        int     lock_state(bool *lock_acquired) const;
        void    unlock_state(bool lock_acquired) const;

        int     lock(pthread_t thread_id) const;
        int     unlock(pthread_t thread_id) const;
        int     try_lock(pthread_t thread_id) const;
        int     try_lock_until(pthread_t thread_id, const struct timespec &absolute_time) const;
        int     try_lock_for(pthread_t thread_id, const struct timespec &relative_time) const;

        bool    is_owned_by_thread(pthread_t thread_id) const;

        int     get_error() const;
        const char *get_error_str() const;
};

static_assert(!std::is_copy_constructible<pt_mutex>::value, "pt_mutex cannot be copied");
static_assert(!std::is_copy_assignable<pt_mutex>::value, "pt_mutex cannot be copy assigned");

#endif
