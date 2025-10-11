#ifndef PTHREAD_THREAD_HPP
#define PTHREAD_THREAD_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "pthread.hpp"
#include <utility>
#include <atomic>
#include <new>
#include "../Template/function.hpp"
#include "../Template/invoke.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/cma_internal.hpp"

class ft_thread
{
    private:
        struct start_data
        {
            ft_function<void()> function;
            std::atomic<int> reference_count;
        };

        pthread_t _thread;
        bool _joinable;
        mutable int _error_code;
        start_data *_start_data;

        static void *start_routine(void *data);
        static void release_start_data(start_data *data);
        void set_error(int error) const;

    public:
        ft_thread();
        template <typename FunctionType, typename... Args>
        ft_thread(FunctionType function, Args... args);
        ~ft_thread();

        ft_thread(const ft_thread &) = delete;
        ft_thread &operator=(const ft_thread &) = delete;
        ft_thread(ft_thread &&other);
        ft_thread &operator=(ft_thread &&other);

        bool joinable() const;
        void join();
        void detach();

        int get_error() const;
        const char *get_error_str() const;
};

template <typename FunctionType, typename... Args>
ft_thread::ft_thread(FunctionType function, Args... args)
    : _thread(), _joinable(false), _error_code(ER_SUCCESS), _start_data(ft_nullptr)
{
    start_data *data;
    void *mutex_address;
    void *function_address;

    data = new (std::nothrow) start_data;
    if (!data)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    data->reference_count.store(2);
    this->_start_data = data;
    mutex_address = data->function.get_mutex_address_debug();
    function_address = reinterpret_cast<void *>(&data->function);
    cma_debug_log_start_data_event("start_data_allocate", data,
        mutex_address, function_address);
    data->function = ft_function<void()>([function, args...]() mutable
    {
        ft_invoke(function, args...);
        return ;
    });
    if (data->function.get_error() != ER_SUCCESS)
    {
        this->set_error(data->function.get_error());
        cma_debug_log_start_data_event("start_data_destroy_function_error",
            data, mutex_address, function_address);
        ft_thread::release_start_data(data);
        ft_thread::release_start_data(data);
        this->_start_data = ft_nullptr;
        return ;
    }
    if (pt_thread_create(&this->_thread, ft_nullptr,
            &ft_thread::start_routine, data) != 0)
    {
        this->set_error(ft_errno);
        cma_debug_log_start_data_event("start_data_destroy_thread_create_fail",
            data, mutex_address, function_address);
        ft_thread::release_start_data(data);
        ft_thread::release_start_data(data);
        this->_start_data = ft_nullptr;
        return ;
    }
    this->_joinable = true;
    return ;
}

#endif
