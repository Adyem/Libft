#ifndef PTHREAD_MUTEX_HPP
# define PTHREAD_MUTEX_HPP

#include <pthread.h>
#include <atomic>
#include <time.h>
#include <type_traits>
#include "../Errno/errno_internal.hpp"

class pt_mutex
{
    private:
        mutable std::atomic<pthread_t>    _owner;
        mutable std::atomic<bool>         _lock;
        mutable pthread_mutex_t           _native_mutex;
        mutable bool                      _native_initialized;
        mutable pt_mutex                  *_state_mutex;
        static thread_local ft_operation_error_stack _operation_errors;

        bool    ensure_native_mutex() const;
        int     lock_internal(bool *lock_acquired) const;
        int     unlock_internal(bool lock_acquired) const;
        void    teardown_thread_safety();

        pt_mutex(const pt_mutex&) = delete;
        pt_mutex& operator=(const pt_mutex&) = delete;
        pt_mutex(pt_mutex&&) = delete;
        pt_mutex& operator=(pt_mutex&&) = delete;

    public:
        pt_mutex();
        ~pt_mutex();

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

        static unsigned long long operation_error_push_entry(int error_code);
        static unsigned long long operation_error_push_entry_with_id(int error_code,
                unsigned long long operation_id);
        static void operation_error_push(int error_code);
        static int operation_error_pop_last();
        static int operation_error_pop_newest();
        static void operation_error_pop_all();
        static int operation_error_error_at(ft_size_t index);
        static int operation_error_last_error();
        static ft_size_t operation_error_depth();
        static unsigned long long operation_error_get_id_at(ft_size_t index);
        static ft_size_t operation_error_find_by_id(unsigned long long operation_id);
        static const char *operation_error_error_str_at(ft_size_t index);
        static const char *operation_error_last_error_str();
};

static_assert(!std::is_copy_constructible<pt_mutex>::value, "pt_mutex cannot be copied");
static_assert(!std::is_copy_assignable<pt_mutex>::value, "pt_mutex cannot be copy assigned");

#endif
