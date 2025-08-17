#ifndef PTHREAD_HPP
# define PTHREAD_HPP

#include <pthread.h>
#include "../Template/promise.hpp"
#include "../CPP_class/nullptr.hpp"
#include <utility>

int pt_thread_join(pthread_t thread, void **retval);
int pt_thread_create(pthread_t *thread, const pthread_attr_t *attr,
                void *(*start_routine)(void *), void *arg);
int pt_thread_detach(pthread_t thread);
int pt_thread_sleep(unsigned int milliseconds);

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
        return ft_nullptr;
    };

    AsyncData* data = new AsyncData{&promise, ft_move(function)};
    pthread_t thread;
    int ret = pt_thread_create(&thread, ft_nullptr, start_routine, data);
    if (ret != 0)
    {
        delete data;
        return ret;
    }
    pt_thread_detach(thread);
    return ret;
}

#endif
