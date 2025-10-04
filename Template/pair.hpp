#ifndef PAIR_HPP
# define PAIR_HPP

#include "move.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"
#include <stdint.h>

template <typename KeyType, typename ValueType>
class Pair
{
    private:
        mutable pt_mutex            _mutex;
        mutable int                 _error_code;

        void set_error(int error) const;

    public:
        KeyType key;
        ValueType value;
        Pair();
        Pair(const KeyType &key, const ValueType &value);
        Pair(const KeyType &key, ValueType &&value);
        Pair(const Pair &other);
        Pair(Pair &&other);
        ~Pair();

        Pair &operator=(const Pair &other);
        Pair &operator=(Pair &&other);

        KeyType get_key() const;
        ValueType get_value() const;
        void set_key(const KeyType &key);
        void set_key(KeyType &&key);
        void set_value(const ValueType &value);
        void set_value(ValueType &&value);
        int get_error() const;
        const char *get_error_str() const;
};

template <typename KeyType, typename ValueType>
void Pair<KeyType, ValueType>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType>::Pair()
        : _mutex(), _error_code(ER_SUCCESS), key(), value()
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType>::Pair(const KeyType &key, const ValueType &value)
        : _mutex(), _error_code(ER_SUCCESS), key(key), value(value)
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType>::Pair(const KeyType &key, ValueType &&value)
        : _mutex(), _error_code(ER_SUCCESS), key(key), value(ft_move(value))
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType>::Pair(const Pair &other)
        : _mutex(), _error_code(ER_SUCCESS), key(), value()
{
    ft_unique_lock<pt_mutex> guard(other._mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->key = other.key;
    this->value = other.value;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType>::Pair(Pair &&other)
        : _mutex(), _error_code(ER_SUCCESS), key(), value()
{
    ft_unique_lock<pt_mutex> guard(other._mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->key = ft_move(other.key);
    this->value = ft_move(other.value);
    other.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType>::~Pair()
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType> &Pair<KeyType, ValueType>::operator=(const Pair &other)
{
    if (this == &other)
    {
        this->set_error(ER_SUCCESS);
        return (*this);
    }
    uintptr_t this_address;
    uintptr_t other_address;
    const Pair<KeyType, ValueType> *first;
    const Pair<KeyType, ValueType> *second;

    this_address = reinterpret_cast<uintptr_t>(this);
    other_address = reinterpret_cast<uintptr_t>(&other);
    if (this_address < other_address)
    {
        first = this;
        second = &other;
    }
    else
    {
        first = &other;
        second = this;
    }
    ft_unique_lock<pt_mutex> first_guard(first->_mutex);
    if (first_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(first_guard.get_error());
        return (*this);
    }
    ft_unique_lock<pt_mutex> second_guard(second->_mutex);
    if (second_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(second_guard.get_error());
        return (*this);
    }
    this->key = other.key;
    this->value = other.value;
    this->set_error(ER_SUCCESS);
    return (*this);
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType> &Pair<KeyType, ValueType>::operator=(Pair &&other)
{
    if (this == &other)
    {
        this->set_error(ER_SUCCESS);
        return (*this);
    }
    uintptr_t this_address;
    uintptr_t other_address;
    const Pair<KeyType, ValueType> *first;
    const Pair<KeyType, ValueType> *second;

    this_address = reinterpret_cast<uintptr_t>(this);
    other_address = reinterpret_cast<uintptr_t>(&other);
    if (this_address < other_address)
    {
        first = this;
        second = &other;
    }
    else
    {
        first = &other;
        second = this;
    }
    ft_unique_lock<pt_mutex> first_guard(first->_mutex);
    if (first_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(first_guard.get_error());
        return (*this);
    }
    ft_unique_lock<pt_mutex> second_guard(second->_mutex);
    if (second_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(second_guard.get_error());
        return (*this);
    }
    this->key = ft_move(other.key);
    this->value = ft_move(other.value);
    other.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return (*this);
}

template <typename KeyType, typename ValueType>
KeyType Pair<KeyType, ValueType>::get_key() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (KeyType());
    }
    KeyType key_copy(this->key);
    this->set_error(ER_SUCCESS);
    return (key_copy);
}

template <typename KeyType, typename ValueType>
ValueType Pair<KeyType, ValueType>::get_value() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (ValueType());
    }
    ValueType value_copy(this->value);
    this->set_error(ER_SUCCESS);
    return (value_copy);
}

template <typename KeyType, typename ValueType>
void Pair<KeyType, ValueType>::set_key(const KeyType &key)
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->key = key;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename KeyType, typename ValueType>
void Pair<KeyType, ValueType>::set_key(KeyType &&key)
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->key = ft_move(key);
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename KeyType, typename ValueType>
void Pair<KeyType, ValueType>::set_value(const ValueType &value)
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->value = value;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename KeyType, typename ValueType>
void Pair<KeyType, ValueType>::set_value(ValueType &&value)
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->value = ft_move(value);
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename KeyType, typename ValueType>
int Pair<KeyType, ValueType>::get_error() const
{
    return (this->_error_code);
}

template <typename KeyType, typename ValueType>
const char *Pair<KeyType, ValueType>::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

template <typename KeyType, typename ValueType>
Pair<KeyType, ValueType> ft_make_pair(KeyType key, ValueType value)
{
    return (Pair<KeyType, ValueType>(key, value));
}

#endif
