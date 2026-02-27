#ifndef CONDITION_HPP
# define CONDITION_HPP

#include <pthread.h>
#include <ctime>

class pt_mutex;

int pt_cond_init(pthread_cond_t *condition, const pthread_condattr_t *attributes);
int pt_cond_destroy(pthread_cond_t *condition);
int pt_cond_wait(pthread_cond_t *condition, pthread_mutex_t *mutex);
int pt_cond_signal(pthread_cond_t *condition);
int pt_cond_broadcast(pthread_cond_t *condition);

class pt_condition_variable
{
    private:
        pthread_cond_t _condition;
        pthread_mutex_t _mutex;
        bool _condition_initialized;
        bool _mutex_initialized;
        mutable pt_mutex *_state_mutex;

        int lock_internal(bool *lock_acquired) const;
        void unlock_internal(bool lock_acquired) const;
        void teardown_thread_safety();

    public:
        pt_condition_variable();
        ~pt_condition_variable();

        pt_condition_variable(const pt_condition_variable&) = delete;
        pt_condition_variable &operator=(const pt_condition_variable&) = delete;

        int enable_thread_safety();
        int disable_thread_safety();
        bool is_thread_safe() const;
        int lock_state(bool *lock_acquired) const;
        void unlock_state(bool lock_acquired) const;

        int wait(pt_mutex &mutex);
        int wait_for(pt_mutex &mutex, const struct timespec &relative_time);
        int wait_until(pt_mutex &mutex, const struct timespec &absolute_time);
        int signal();
        int broadcast();
};

#endif
