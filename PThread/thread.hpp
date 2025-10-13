#ifndef PTHREAD_THREAD_HPP
#define PTHREAD_THREAD_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "pthread.hpp"
#include <memory>
#include <new>
#include <utility>
#include "../Template/function.hpp"
#include "../Template/invoke.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/cma_internal.hpp"

class ft_thread
{
    private:
        struct start_payload
        {
            ft_function<void()> function;

            start_payload();
            ~start_payload();
        };

        pthread_t _thread;
        bool _joinable;
        mutable int _error_code;
        std::shared_ptr<start_payload> _start_payload;

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
    : _thread(), _joinable(false), _error_code(ER_SUCCESS), _start_payload()
{
    start_payload *payload_raw;
    std::shared_ptr<start_payload> payload;
    std::shared_ptr<start_payload> *shared_capsule;

    payload_raw = new (std::nothrow) start_payload();
    if (!payload_raw)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    payload = std::shared_ptr<start_payload>(payload_raw);
    this->_start_payload = payload;
    payload->function = ft_function<void()>([function, args...]() mutable
    {
        ft_invoke(function, args...);
        return ;
    });
    if (payload->function.get_error() != ER_SUCCESS)
    {
        this->set_error(payload->function.get_error());
        this->_start_payload.reset();
        return ;
    }
    shared_capsule = new (std::nothrow) std::shared_ptr<start_payload>(payload);
    if (!shared_capsule)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        this->_start_payload.reset();
        return ;
    }
    if (pt_thread_create(&this->_thread, ft_nullptr,
            &ft_thread::start_routine, shared_capsule) != 0)
    {
        this->set_error(ft_errno);
        delete shared_capsule;
        this->_start_payload.reset();
        return ;
    }
    this->_joinable = true;
    return ;
}

#endif
