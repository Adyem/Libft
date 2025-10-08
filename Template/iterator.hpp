 #ifndef ITERATOR_HPP
 # define ITERATOR_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

template <typename ValueType>
class Iterator
{
    private:
        ValueType* _ptr;
        mutable int _error_code;

        void set_error(int error_code) const noexcept;

    public:
        Iterator(ValueType* ptr) noexcept;
        Iterator(const Iterator& other) noexcept;
        Iterator& operator=(const Iterator& other) noexcept;
        Iterator(Iterator&& other) noexcept;
        Iterator& operator=(Iterator&& other) noexcept;
        ~Iterator() = default;
        Iterator operator++() noexcept;
        bool operator!=(const Iterator& other) const noexcept;
        ValueType& operator*() const noexcept;
        int get_error() const noexcept;
        const char* get_error_str() const noexcept;
};

template <typename ValueType>
Iterator<ValueType>::Iterator(ValueType* ptr) noexcept
    : _ptr(ptr), _error_code(ER_SUCCESS)
{
    if (this->_ptr == ft_nullptr)
        this->set_error(FT_ERR_INVALID_ARGUMENT);
    else
        this->set_error(ER_SUCCESS);
    return ;
}

template <typename ValueType>
Iterator<ValueType>::Iterator(const Iterator& other) noexcept
    : _ptr(other._ptr), _error_code(other._error_code)
{
    this->set_error(this->_error_code);
    return ;
}

template <typename ValueType>
Iterator<ValueType>& Iterator<ValueType>::operator=(const Iterator& other) noexcept
{
    if (this != &other)
    {
        this->_ptr = other._ptr;
        this->_error_code = other._error_code;
    }
    this->set_error(this->_error_code);
    return (*this);
}

template <typename ValueType>
Iterator<ValueType>::Iterator(Iterator&& other) noexcept
    : _ptr(other._ptr), _error_code(other._error_code)
{
    other._ptr = ft_nullptr;
    other._error_code = ER_SUCCESS;
    this->set_error(this->_error_code);
    return ;
}

template <typename ValueType>
Iterator<ValueType>& Iterator<ValueType>::operator=(Iterator&& other) noexcept
{
    if (this != &other)
    {
        this->_ptr = other._ptr;
        this->_error_code = other._error_code;
        other._ptr = ft_nullptr;
        other._error_code = ER_SUCCESS;
    }
    this->set_error(this->_error_code);
    return (*this);
}

template <typename ValueType>
Iterator<ValueType> Iterator<ValueType>::operator++() noexcept
{
    if (this->_ptr == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (*this);
    }
    ++this->_ptr;
    this->set_error(ER_SUCCESS);
    return (*this);
}

template <typename ValueType>
bool Iterator<ValueType>::operator!=(const Iterator& other) const noexcept
{
    bool result = (this->_ptr != other._ptr);
    this->set_error(ER_SUCCESS);
    return (result);
}

template <typename ValueType>
ValueType& Iterator<ValueType>::operator*() const noexcept
{
    if (this->_ptr == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        static ValueType default_value = ValueType();
        return (default_value);
    }
    this->set_error(ER_SUCCESS);
    return (*this->_ptr);
}

template <typename ValueType>
int Iterator<ValueType>::get_error() const noexcept
{
    return (this->_error_code);
}

template <typename ValueType>
const char* Iterator<ValueType>::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

template <typename ValueType>
void Iterator<ValueType>::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

#endif
