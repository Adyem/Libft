 #ifndef ITERATOR_HPP
 # define ITERATOR_HPP

#include "../CPP_class/cpp_class_nullptr.hpp"

template <typename ValueType>
class Iterator
{
    public:
        Iterator(ValueType* ptr);
        Iterator(const Iterator& other);
        Iterator& operator=(const Iterator& other);
        Iterator(Iterator&& other) noexcept;
        Iterator& operator=(Iterator&& other) noexcept;
        ~Iterator() = default;
        Iterator operator++();
        bool operator!=(const Iterator& other) const;
        ValueType& operator*() const;

    private:
        ValueType* _ptr;
};

template <typename ValueType>
Iterator<ValueType>::Iterator(ValueType* ptr) : _ptr(ptr)
{
    return ;
}

template <typename ValueType>
Iterator<ValueType>::Iterator(const Iterator& other) : _ptr(other._ptr)
{
    return ;
}

template <typename ValueType>
Iterator<ValueType>& Iterator<ValueType>::operator=(const Iterator& other)
{
    if (this != &other)
        _ptr = other._ptr;
    return (*this);
}

template <typename ValueType>
Iterator<ValueType>::Iterator(Iterator&& other) noexcept : _ptr(other._ptr)
{
    other._ptr = ft_nullptr;
        return ;
}

template <typename ValueType>
Iterator<ValueType>& Iterator<ValueType>::operator=(Iterator&& other) noexcept
{
    if (this != &other)
    {
        _ptr = other._ptr;
        other._ptr = ft_nullptr;
    }
    return (*this);
}

template <typename ValueType>
Iterator<ValueType> Iterator<ValueType>::operator++()
{
    ++_ptr;
    return (*this);
}

template <typename ValueType>
bool Iterator<ValueType>::operator!=(const Iterator& other) const
{
    return (_ptr != other._ptr);
}

template <typename ValueType>
ValueType& Iterator<ValueType>::operator*() const
{
    return (*_ptr);
}

#endif
