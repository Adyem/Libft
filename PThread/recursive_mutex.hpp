#ifndef RECURSIVE_MUTEX_HPP
# define RECURSIVE_MUTEX_HPP

#include <pthread.h>
#include <atomic>
#include <type_traits>
#include <cstddef>
#include <mutex>
#include <cstdint>

#include "mutex.hpp"

class pt_recursive_mutex
{
    private:
        mutable std::atomic<pt_thread_id_type>    _owner;
        mutable std::atomic<bool>         _lock;
        mutable std::atomic<std::size_t>  _lock_depth;
        mutable std::mutex               *_native_mutex;
        mutable uint8_t                   _initialized_state;
        mutable pt_mutex                  *_state_mutex;
        mutable std::atomic<bool>         _valid_state;
        static const uint8_t              _state_uninitialized = 0;
        static const uint8_t              _state_destroyed = 1;
        static const uint8_t              _state_initialized = 2;

        int     ensure_native_mutex() const;
        int     lock_internal(bool *lock_acquired) const;
        int     unlock_internal(bool lock_acquired) const;
        void    teardown_thread_safety();
        void    abort_lifecycle_error(const char *method_name,
                    const char *reason) const;
        void    abort_if_not_initialized(const char *method_name) const;

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

        int     lock() const;
        int     unlock() const;
        int     try_lock(pt_thread_id_type thread_id) const;

        bool    is_owned_by_thread(pt_thread_id_type thread_id) const;

        int     initialize();
        int     destroy();
};

static_assert(!std::is_copy_constructible<pt_recursive_mutex>::value, "pt_recursive_mutex cannot be copied");
static_assert(!std::is_copy_assignable<pt_recursive_mutex>::value, "pt_recursive_mutex cannot be copy assigned");

#endif
