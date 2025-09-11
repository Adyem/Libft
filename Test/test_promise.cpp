#include <utility>
#ifndef ft_move
# define ft_move std::move
#endif
#include "../Template/promise.hpp"
#include "../PThread/pthread.hpp"
#include "../Errno/errno.hpp"
#include <unistd.h>

int test_ft_promise_set_get(void)
{
    ft_promise<int> p;
    p.set_value(42);
    return (p.is_ready() && p.get() == 42 && p.get_error() == ER_SUCCESS);
}

int test_ft_promise_not_ready(void)
{
    ft_promise<int> p;
    p.get();
    return (p.get_error() == FT_EINVAL);
}

int test_pt_async_basic(void)
{
    ft_promise<int> p;
    if (pt_async(p, []() { return (7); }) != 0)
        return (0);
    while (!p.is_ready())
        usleep(1000);
    return (p.get() == 7);
}

int test_pt_cond_wait_signal(void)
{
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    pthread_mutex_init(&mutex, ft_nullptr);
    pt_cond_init(&condition, ft_nullptr);
    int ready = 0;
    struct condition_data
    {
        pthread_mutex_t *mutex;
        pthread_cond_t *condition;
        int *ready;
    };
    condition_data data = { &mutex, &condition, &ready };
    auto start_routine = [](void *arg) -> void*
    {
        condition_data *data = static_cast<condition_data*>(arg);
        pthread_mutex_lock(data->mutex);
        while (*(data->ready) == 0)
            pt_cond_wait(data->condition, data->mutex);
        pthread_mutex_unlock(data->mutex);
        return (ft_nullptr);
    };
    pthread_t thread;
    if (pt_thread_create(&thread, ft_nullptr, start_routine, &data) != 0)
    {
        pt_cond_destroy(&condition);
        pthread_mutex_destroy(&mutex);
        return (0);
    }
    pt_thread_sleep(100);
    pthread_mutex_lock(&mutex);
    ready = 1;
    pt_cond_signal(&condition);
    pthread_mutex_unlock(&mutex);
    int join_result = pt_thread_join(thread, ft_nullptr);
    pt_cond_destroy(&condition);
    pthread_mutex_destroy(&mutex);
    return (join_result == 0 && ready == 1);
}
