#ifndef TEMPLATE_FUNCTION_HPP
#define TEMPLATE_FUNCTION_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <utility>
#include <new>
#include <type_traits>
#include <stdint.h>

template <typename Signature>
class ft_function;

template <typename ReturnType, typename... Args>
class ft_function<ReturnType(Args...)>
{
    private:
        void *_callable;
        ReturnType (*_invoke)(void *, Args...);
        void (*_destroy)(void *);
        void *(*_clone)(void *);
        mutable int _error_code;

        void set_error(int error) const;
        void clear_callable();

        template <typename FunctionType>
        static ReturnType invoke(void *callable, Args... args);

        template <typename FunctionType>
        static void destroy(void *callable);

        template <typename FunctionType>
        static void *clone(void *callable);

    public:
        ft_function();

        template <typename FunctionType>
        ft_function(FunctionType function);

        ft_function(const ft_function &other);
        ft_function(ft_function &&other);
        ~ft_function();

        ft_function &operator=(const ft_function &other);
        ft_function &operator=(ft_function &&other);

        ReturnType operator()(Args... args) const;
        explicit operator bool() const;

        int get_error() const;
        const char *get_error_str() const;
        void *get_mutex_address_debug() const;
};

template <typename ReturnType, typename... Args>
void ft_function<ReturnType(Args...)>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename ReturnType, typename... Args>
void ft_function<ReturnType(Args...)>::clear_callable()
{
    if (this->_destroy && this->_callable)
    {
        this->_destroy(this->_callable);
    }
    this->_callable = ft_nullptr;
    this->_invoke = ft_nullptr;
    this->_destroy = ft_nullptr;
    this->_clone = ft_nullptr;
    return ;
}

template <typename ReturnType, typename... Args>
template <typename FunctionType>
ReturnType ft_function<ReturnType(Args...)>::invoke(void *callable, Args... args)
{
    FunctionType *function;

    function = static_cast<FunctionType *>(callable);
    return ((*function)(args...));
}

template <typename ReturnType, typename... Args>
template <typename FunctionType>
void ft_function<ReturnType(Args...)>::destroy(void *callable)
{
    FunctionType *function;

    function = static_cast<FunctionType *>(callable);
    delete function;
    return ;
}

template <typename ReturnType, typename... Args>
template <typename FunctionType>
void *ft_function<ReturnType(Args...)>::clone(void *callable)
{
    FunctionType *function;
    FunctionType *copy;

    function = static_cast<FunctionType *>(callable);
    copy = new (std::nothrow) FunctionType(*function);
    return (copy);
}

template <typename ReturnType, typename... Args>
ft_function<ReturnType(Args...)>::ft_function()
    : _callable(ft_nullptr), _invoke(ft_nullptr), _destroy(ft_nullptr),
      _clone(ft_nullptr), _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ReturnType, typename... Args>
template <typename FunctionType>
ft_function<ReturnType(Args...)>::ft_function(FunctionType function)
    : _callable(ft_nullptr), _invoke(ft_nullptr), _destroy(ft_nullptr),
      _clone(ft_nullptr), _error_code(ER_SUCCESS)
{
    FunctionType *copy;

    copy = new (std::nothrow) FunctionType(std::move(function));
    if (!copy)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    this->_callable = copy;
    this->_invoke = &ft_function<ReturnType(Args...)>::template invoke<FunctionType>;
    this->_destroy = &ft_function<ReturnType(Args...)>::template destroy<FunctionType>;
    this->_clone = &ft_function<ReturnType(Args...)>::template clone<FunctionType>;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ReturnType, typename... Args>
ft_function<ReturnType(Args...)>::ft_function(const ft_function &other)
    : _callable(ft_nullptr), _invoke(ft_nullptr), _destroy(ft_nullptr),
      _clone(ft_nullptr), _error_code(ER_SUCCESS)
{
    if (other._callable)
    {
        this->_callable = other._clone(other._callable);
        if (!this->_callable)
        {
            this->set_error(FT_ERR_NO_MEMORY);
            return ;
        }
    }
    this->_invoke = other._invoke;
    this->_destroy = other._destroy;
    this->_clone = other._clone;
    this->_error_code = other._error_code;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ReturnType, typename... Args>
ft_function<ReturnType(Args...)>::ft_function(ft_function &&other)
    : _callable(other._callable), _invoke(other._invoke),
      _destroy(other._destroy), _clone(other._clone), _error_code(other._error_code)
{
    other._callable = ft_nullptr;
    other._invoke = ft_nullptr;
    other._destroy = ft_nullptr;
    other._clone = ft_nullptr;
    other.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ReturnType, typename... Args>
ft_function<ReturnType(Args...)>::~ft_function()
{
    this->clear_callable();
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ReturnType, typename... Args>
ft_function<ReturnType(Args...)> &ft_function<ReturnType(Args...)>::operator=(const ft_function &other)
{
    if (this == &other)
    {
        this->set_error(ER_SUCCESS);
        return (*this);
    }
    void *new_callable;

    new_callable = ft_nullptr;
    if (other._callable)
    {
        new_callable = other._clone(other._callable);
        if (!new_callable)
        {
            this->set_error(FT_ERR_NO_MEMORY);
            return (*this);
        }
    }
    this->clear_callable();
    this->_callable = new_callable;
    this->_invoke = other._invoke;
    this->_destroy = other._destroy;
    this->_clone = other._clone;
    this->_error_code = other._error_code;
    this->set_error(ER_SUCCESS);
    return (*this);
}

template <typename ReturnType, typename... Args>
ft_function<ReturnType(Args...)> &ft_function<ReturnType(Args...)>::operator=(ft_function &&other)
{
    if (this == &other)
    {
        this->set_error(ER_SUCCESS);
        return (*this);
    }
    this->clear_callable();
    this->_callable = other._callable;
    this->_invoke = other._invoke;
    this->_destroy = other._destroy;
    this->_clone = other._clone;
    this->_error_code = other._error_code;
    other._callable = ft_nullptr;
    other._invoke = ft_nullptr;
    other._destroy = ft_nullptr;
    other._clone = ft_nullptr;
    other.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return (*this);
}

template <typename ReturnType, typename... Args>
ReturnType ft_function<ReturnType(Args...)>::operator()(Args... args) const
{
    ReturnType (*invoke_target)(void *, Args...);
    void *callable;

    invoke_target = this->_invoke;
    callable = this->_callable;
    if (!invoke_target || !callable)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        if constexpr (std::is_void<ReturnType>::value)
        {
            return ;
        }
        return (ReturnType());
    }
    this->set_error(ER_SUCCESS);
    if constexpr (std::is_void<ReturnType>::value)
    {
        invoke_target(callable, args...);
        return ;
    }
    return (invoke_target(callable, args...));
}

template <typename ReturnType, typename... Args>
ft_function<ReturnType(Args...)>::operator bool() const
{
    bool has_callable;
    has_callable = false;
    if (this->_callable && this->_invoke)
    {
        has_callable = true;
    }
    this->set_error(ER_SUCCESS);
    return (has_callable);
}

template <typename ReturnType, typename... Args>
int ft_function<ReturnType(Args...)>::get_error() const
{
    return (this->_error_code);
}

template <typename ReturnType, typename... Args>
const char *ft_function<ReturnType(Args...)>::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

template <typename ReturnType, typename... Args>
void *ft_function<ReturnType(Args...)>::get_mutex_address_debug() const
{
    return (ft_nullptr);
}

#endif
