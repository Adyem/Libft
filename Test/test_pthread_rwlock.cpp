#include "../PThread/pthread.hpp"
#include "../Errno/errno.hpp"

struct reader_data
{
    pthread_rwlock_t *lock;
    int *started;
};

struct writer_data
{
    pthread_rwlock_t *lock;
    int *acquired;
    int *shared;
};

static void *reader_routine(void *arg)
{
    reader_data *data = static_cast<reader_data*>(arg);
    pt_rwlock_rdlock(data->lock);
    *(data->started) = 1;
    pt_thread_sleep(200);
    pt_rwlock_unlock(data->lock);
    return (ft_nullptr);
}

static void *writer_routine(void *arg)
{
    writer_data *data = static_cast<writer_data*>(arg);
    pt_rwlock_wrlock(data->lock);
    *(data->shared) = 1;
    *(data->acquired) = 1;
    pt_rwlock_unlock(data->lock);
    return (ft_nullptr);
}

int test_pt_rwlock_readers_writers(void)
{
    pthread_rwlock_t rwlock;
    if (pt_rwlock_init(&rwlock, ft_nullptr) != 0)
        return (0);
    int shared_value = 0;
    int reader_one_started = 0;
    int reader_two_started = 0;
    int writer_acquired = 0;
    reader_data data_one = { &rwlock, &reader_one_started };
    reader_data data_two = { &rwlock, &reader_two_started };
    pthread_t reader_one;
    pthread_t reader_two;
    if (pt_thread_create(&reader_one, ft_nullptr, reader_routine, &data_one) != 0)
    {
        pt_rwlock_destroy(&rwlock);
        return (0);
    }
    if (pt_thread_create(&reader_two, ft_nullptr, reader_routine, &data_two) != 0)
    {
        pt_thread_join(reader_one, ft_nullptr);
        pt_rwlock_destroy(&rwlock);
        return (0);
    }
    while (reader_one_started == 0 || reader_two_started == 0)
        pt_thread_sleep(10);
    writer_data wdata = { &rwlock, &writer_acquired, &shared_value };
    pthread_t writer_thread;
    if (pt_thread_create(&writer_thread, ft_nullptr, writer_routine, &wdata) != 0)
    {
        pt_thread_join(reader_one, ft_nullptr);
        pt_thread_join(reader_two, ft_nullptr);
        pt_rwlock_destroy(&rwlock);
        return (0);
    }
    pt_thread_sleep(100);
    int writer_acquired_early = writer_acquired;
    pt_thread_join(reader_one, ft_nullptr);
    pt_thread_join(reader_two, ft_nullptr);
    pt_thread_join(writer_thread, ft_nullptr);
    pt_rwlock_destroy(&rwlock);
    return (writer_acquired == 1 && writer_acquired_early == 0 && shared_value == 1);
}

