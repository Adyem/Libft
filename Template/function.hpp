#ifndef TEMPLATE_FUNCTION_HPP
#define TEMPLATE_FUNCTION_HPP

#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "move.hpp"
#include <new>

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

        template <typename FunctionType>
        static ReturnType invoke(void *callable, Args... args)
        {
            FunctionType *function;

            function = static_cast<FunctionType *>(callable);
            return ((*function)(args...));
        }

        template <typename FunctionType>
        static void destroy(void *callable)
        {
            FunctionType *function;

            function = static_cast<FunctionType *>(callable);
            delete function;
        }

        template <typename FunctionType>
        static void *clone(void *callable)
        {
            FunctionType *function;
            FunctionType *copy;

            function = static_cast<FunctionType *>(callable);
            copy = new (std::nothrow) FunctionType(*function);
            return (copy);
        }

        void set_error(int error) const
        {
            this->_error_code = error;
            ft_errno = error;
        }

    public:
        ft_function()
            : _callable(ft_nullptr), _invoke(ft_nullptr), _destroy(ft_nullptr),
              _clone(ft_nullptr), _error_code(ER_SUCCESS)
        {
            this->set_error(ER_SUCCESS);
            return ;
        }

        template <typename FunctionType>
        ft_function(FunctionType function)
            : _callable(ft_nullptr), _invoke(ft_nullptr), _destroy(ft_nullptr),
              _clone(ft_nullptr), _error_code(ER_SUCCESS)
        {
            FunctionType *copy;

            copy = new (std::nothrow) FunctionType(ft_move(function));
            if (!copy)
            {
                this->set_error(FT_EALLOC);
                return ;
            }
            this->_callable = copy;
            this->_invoke = &ft_function::invoke<FunctionType>;
            this->_destroy = &ft_function::destroy<FunctionType>;
            this->_clone = &ft_function::clone<FunctionType>;
            this->set_error(ER_SUCCESS);
            return ;
        }

        ft_function(const ft_function &other)
            : _callable(ft_nullptr), _invoke(other._invoke),
              _destroy(other._destroy), _clone(other._clone),
              _error_code(other._error_code)
        {
            if (other._callable)
            {
                this->_callable = other._clone(other._callable);
                if (!this->_callable)
                {
                    this->set_error(FT_EALLOC);
                    return ;
                }
            }
            this->set_error(ER_SUCCESS);
            return ;
        }

        ft_function(ft_function &&other)
            : _callable(other._callable), _invoke(other._invoke),
              _destroy(other._destroy), _clone(other._clone),
              _error_code(other._error_code)
        {
            other._callable = ft_nullptr;
            other._invoke = ft_nullptr;
            other._destroy = ft_nullptr;
            other._clone = ft_nullptr;
            other._error_code = ER_SUCCESS;
            this->set_error(ER_SUCCESS);
            return ;
        }

        ft_function &operator=(const ft_function &other)
        {
            if (this != &other)
            {
                if (this->_destroy)
                    this->_destroy(this->_callable);
                this->_callable = ft_nullptr;
                this->_invoke = other._invoke;
                this->_destroy = other._destroy;
                this->_clone = other._clone;
                this->_error_code = other._error_code;
                if (other._callable)
                {
                    this->_callable = other._clone(other._callable);
                    if (!this->_callable)
                    {
                        this->set_error(FT_EALLOC);
                        return (*this);
                    }
                }
                this->set_error(ER_SUCCESS);
            }
            return (*this);
        }

        ft_function &operator=(ft_function &&other)
        {
            if (this != &other)
            {
                if (this->_destroy)
                    this->_destroy(this->_callable);
                this->_callable = other._callable;
                this->_invoke = other._invoke;
                this->_destroy = other._destroy;
                this->_clone = other._clone;
                this->_error_code = other._error_code;
                other._callable = ft_nullptr;
                other._invoke = ft_nullptr;
                other._destroy = ft_nullptr;
                other._clone = ft_nullptr;
                other._error_code = ER_SUCCESS;
                this->set_error(ER_SUCCESS);
            }
            return (*this);
        }

        ~ft_function()
        {
            if (this->_destroy)
                this->_destroy(this->_callable);
            this->set_error(ER_SUCCESS);
            return ;
        }

        ReturnType operator()(Args... args) const
        {
            return (this->_invoke(this->_callable, args...));
        }

        explicit operator bool() const
        {
            return (this->_callable != ft_nullptr);
        }

        int get_error() const
        {
            return (this->_error_code);
        }

        const char *get_error_str() const
        {
            return (ft_strerror(this->_error_code));
        }
};

#endif
