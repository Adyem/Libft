#include "environment_lock.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <cstdlib>
#include <pthread.h>

static pthread_mutex_t *g_environment_mutex = ft_nullptr;

int ft_environment_lock(void)
{
    int pthread_error;

    if (g_environment_mutex == ft_nullptr)
        return (-1);
    pthread_error = pthread_mutex_lock(g_environment_mutex);
    if (pthread_error != 0)
        return (-1);
    return (0);
}

int ft_environment_unlock(void)
{
    int pthread_error;

    if (g_environment_mutex == ft_nullptr)
        return (-1);
    pthread_error = pthread_mutex_unlock(g_environment_mutex);
    if (pthread_error != 0)
        return (-1);
    return (0);
}

int ft_environment_enable_thread_safety(void)
{
    pthread_mutex_t  *mutex_pointer;
    int             pthread_error;

    if (g_environment_mutex != ft_nullptr)
        return (0);
    mutex_pointer = static_cast<pthread_mutex_t *>(std::malloc(sizeof(pthread_mutex_t)));
    if (mutex_pointer == ft_nullptr)
        return (-1);
    pthread_error = pthread_mutex_init(mutex_pointer, ft_nullptr);
    if (pthread_error != 0)
    {
        std::free(mutex_pointer);
        return (-1);
    }
    g_environment_mutex = mutex_pointer;
    return (0);
}

void ft_environment_disable_thread_safety(void)
{
    if (g_environment_mutex != ft_nullptr)
    {
        pthread_mutex_destroy(g_environment_mutex);
        std::free(g_environment_mutex);
        g_environment_mutex = ft_nullptr;
    }
    return ;
}
