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
            std::atomic<bool> owner_finalized;
        };

        pthread_t _thread;
        bool _joinable;
        mutable int _error_code;
        start_data *_start_data;

        static void *start_routine(void *data);
        static void release_start_data(start_data *data, bool owner_release);
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
    data = new (std::nothrow) start_data;
    if (!data)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    data->reference_count.store(2);
    data->owner_finalized.store(false);
    this->_start_data = data;
    data->function = ft_function<void()>([function, args...]() mutable
    {
        ft_invoke(function, args...);
        return ;
    });
    if (data->function.get_error() != ER_SUCCESS)
    {
        this->set_error(data->function.get_error());
        ft_thread::release_start_data(data, false);
        ft_thread::release_start_data(data, true);
        this->_start_data = ft_nullptr;
        return ;
    }
    if (pt_thread_create(&this->_thread, ft_nullptr,
            &ft_thread::start_routine, data) != 0)
    {
        this->set_error(ft_errno);
        ft_thread::release_start_data(data, false);
        ft_thread::release_start_data(data, true);
        this->_start_data = ft_nullptr;
        return ;
    }
    this->_joinable = true;
    return ;
}

#endif
