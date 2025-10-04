#ifndef ATOMIC_HPP
#define ATOMIC_HPP

#include "../Errno/errno.hpp"
#include <atomic>


template <typename ValueType>
class ft_atomic
{
    private:
        std::atomic<ValueType> _value;
        mutable int _error_code;

        void set_error(int error) const;

    public:
        ft_atomic();
        explicit ft_atomic(ValueType desired_value);
        ~ft_atomic();

        ft_atomic(const ft_atomic&) = delete;
        ft_atomic &operator=(const ft_atomic&) = delete;
        ft_atomic(ft_atomic&&) = delete;
        ft_atomic &operator=(ft_atomic&&) = delete;

        void store(ValueType desired_value, std::memory_order order = std::memory_order_seq_cst);
        ValueType load(std::memory_order order = std::memory_order_seq_cst) const;
        bool compare_exchange_weak(ValueType &expected_value, ValueType desired_value,
                std::memory_order success_order = std::memory_order_seq_cst,
                std::memory_order failure_order = std::memory_order_seq_cst);
        bool compare_exchange_strong(ValueType &expected_value, ValueType desired_value,
                std::memory_order success_order = std::memory_order_seq_cst,
                std::memory_order failure_order = std::memory_order_seq_cst);
        ValueType fetch_add(ValueType value, std::memory_order order = std::memory_order_seq_cst);

        std::atomic<ValueType> *native_handle();
        const std::atomic<ValueType> *native_handle() const;

        int get_error() const;
        const char *get_error_str() const;
};


template <typename ValueType>
void ft_atomic<ValueType>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}


template <typename ValueType>
ft_atomic<ValueType>::ft_atomic()
    : _value(), _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}


template <typename ValueType>
ft_atomic<ValueType>::ft_atomic(ValueType desired_value)
    : _value(desired_value), _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}


template <typename ValueType>
ft_atomic<ValueType>::~ft_atomic()
{
    this->set_error(ER_SUCCESS);
    return ;
}


template <typename ValueType>
void ft_atomic<ValueType>::store(ValueType desired_value, std::memory_order order)
{
    this->_value.store(desired_value, order);
    this->set_error(ER_SUCCESS);
    return ;
}


template <typename ValueType>
ValueType ft_atomic<ValueType>::load(std::memory_order order) const
{
    ValueType current_value;

    current_value = this->_value.load(order);
    const_cast<ft_atomic<ValueType> *>(this)->set_error(ER_SUCCESS);
    return (current_value);
}


template <typename ValueType>
bool ft_atomic<ValueType>::compare_exchange_weak(ValueType &expected_value, ValueType desired_value,
        std::memory_order success_order, std::memory_order failure_order)
{
    bool result;

    result = this->_value.compare_exchange_weak(expected_value, desired_value, success_order, failure_order);
    this->set_error(ER_SUCCESS);
    return (result);
}


template <typename ValueType>
bool ft_atomic<ValueType>::compare_exchange_strong(ValueType &expected_value, ValueType desired_value,
        std::memory_order success_order, std::memory_order failure_order)
{
    bool result;

    result = this->_value.compare_exchange_strong(expected_value, desired_value, success_order, failure_order);
    this->set_error(ER_SUCCESS);
    return (result);
}


template <typename ValueType>
ValueType ft_atomic<ValueType>::fetch_add(ValueType value, std::memory_order order)
{
    ValueType previous_value;

    previous_value = this->_value.fetch_add(value, order);
    this->set_error(ER_SUCCESS);
    return (previous_value);
}


template <typename ValueType>
std::atomic<ValueType> *ft_atomic<ValueType>::native_handle()
{
    this->set_error(ER_SUCCESS);
    return (&this->_value);
}


template <typename ValueType>
const std::atomic<ValueType> *ft_atomic<ValueType>::native_handle() const
{
    const_cast<ft_atomic<ValueType> *>(this)->set_error(ER_SUCCESS);
    return (&this->_value);
}


template <typename ValueType>
int ft_atomic<ValueType>::get_error() const
{
    return (this->_error_code);
}


template <typename ValueType>
const char *ft_atomic<ValueType>::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}


#endif
