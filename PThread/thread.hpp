#ifndef PTHREAD_THREAD_HPP
#define PTHREAD_THREAD_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "pthread.hpp"
#include "../Template/move.hpp"
#include "../Template/function.hpp"
#include "../Template/invoke.hpp"
#include "../Errno/errno.hpp"

class ft_thread
{
    private:
        pthread_t _thread;
        bool _joinable;
        mutable int _error_code;

        struct start_data
        {
            ft_function<void()> function;
        };

        static void *start_routine(void *data);
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
    : _thread(), _joinable(false), _error_code(ER_SUCCESS)
{
    start_data *data;

    data = new (std::nothrow) start_data;
    if (!data)
    {
        this->set_error(FT_EALLOC);
        return ;
    }
    data->function = ft_function<void()>([function, args...]() mutable
    {
        ft_invoke(function, args...);
        return ;
    });
    if (data->function.get_error() != ER_SUCCESS)
    {
        this->set_error(data->function.get_error());
        delete data;
        return ;
    }
    if (pt_thread_create(&this->_thread, ft_nullptr,
            &ft_thread::start_routine, data) != 0)
    {
        this->set_error(ft_errno);
        delete data;
        return ;
    }
    this->_joinable = true;
    return ;
}

#endif
