#ifndef PTHREAD_HPP
# define PTHREAD_HPP

#include <pthread.h>
#include "../Template/promise.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Template/atomic.hpp"
#include "condition.hpp"
#include "../Template/move.hpp"
#include "../Time/time.hpp"

int pt_thread_join(pthread_t thread, void **retval);
int pt_thread_create(pthread_t *thread, const pthread_attr_t *attr,
                void *(*start_routine)(void *), void *arg);
int pt_thread_detach(pthread_t thread);
int pt_thread_cancel(pthread_t thread);
int pt_thread_sleep(unsigned int milliseconds);
int pt_thread_yield();
int pt_thread_equal(pthread_t thread1, pthread_t thread2);

int pt_atomic_load(const ft_atomic<int>& atomic_variable);
void pt_atomic_store(ft_atomic<int>& atomic_variable, int desired_value);
int pt_atomic_fetch_add(ft_atomic<int>& atomic_variable, int increment_value);
bool pt_atomic_compare_exchange(ft_atomic<int>& atomic_variable, int& expected_value, int desired_value);

int pt_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attributes);
int pt_rwlock_rdlock(pthread_rwlock_t *rwlock);
int pt_rwlock_wrlock(pthread_rwlock_t *rwlock);
int pt_rwlock_unlock(pthread_rwlock_t *rwlock);
int pt_rwlock_destroy(pthread_rwlock_t *rwlock);

#define SLEEP_TIME 100
#define MAX_SLEEP 10000
#define MAX_QUEUE 128

#ifdef _WIN32
    #include <windows.h>
    using pt_thread_id_type = DWORD;
    #define THREAD_ID GetCurrentThreadId()
#else
    #include <pthread.h>
    using pt_thread_id_type = pthread_t;
    #define THREAD_ID pthread_self()
#endif

extern thread_local pt_thread_id_type pt_thread_id;
pt_thread_id_type pt_thread_self();

typedef struct s_thread_id
{
    pt_thread_id_type native_id;
}   t_thread_id;

t_thread_id    ft_this_thread_get_id();
void    ft_this_thread_sleep_for(t_duration_milliseconds duration);
void    ft_this_thread_sleep_until(t_monotonic_time_point time_point);
void    ft_this_thread_yield();

template <typename ValueType, typename Function>
int pt_async(ft_promise<ValueType>& promise, Function function)
{
    struct AsyncData
    {
        ft_promise<ValueType>* promise;
        Function function;
    };

    auto start_routine = [](void* arg) -> void*
    {
        AsyncData* data = static_cast<AsyncData*>(arg);
        data->promise->set_value(data->function());
        delete data;
        return (ft_nullptr);
    };

    AsyncData* data = new AsyncData{&promise, ft_move(function)};
    pthread_t thread;
    int ret = pt_thread_create(&thread, ft_nullptr, start_routine, data);
    if (ret != 0)
    {
        delete data;
        return (ret);
    }
    pt_thread_detach(thread);
    return (ret);
}

#endif
