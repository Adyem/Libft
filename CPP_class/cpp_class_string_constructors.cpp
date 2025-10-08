#include "class_string_class.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include "class_nullptr.hpp"

ft_string::ft_string() noexcept
    : _data(ft_nullptr), _length(0), _capacity(0), _error_code(0)
{
    this->set_error(ER_SUCCESS);
    return ;
}

ft_string::ft_string(const char* init_str) noexcept
    : _data(ft_nullptr), _length(0), _capacity(0), _error_code(0)
{
    this->set_error(ER_SUCCESS);
    if (init_str)
    {
        this->_length = ft_strlen_size_t(init_str);
        this->_capacity = this->_length;
        this->_data = static_cast<char*>(cma_calloc(this->_capacity + 1, sizeof(char)));
        if (!this->_data)
        {
            this->set_error(FT_ERR_NO_MEMORY);
            return ;
        }
        ft_memcpy(this->_data, init_str, this->_length + 1);
    }
    return ;
}

ft_string::ft_string(size_t count, char character) noexcept
    : _data(ft_nullptr), _length(0), _capacity(0), _error_code(0)
{
    this->set_error(ER_SUCCESS);
    this->assign(count, character);
    return ;
}

ft_string::ft_string(const ft_string& other) noexcept
    : _data(ft_nullptr), _length(other._length), _capacity(other._capacity),
      _error_code(other._error_code)
{
    this->set_error(other._error_code);
    if (other._error_code != ER_SUCCESS)
    {
        this->_length = 0;
        this->_capacity = 0;
        return ;
    }
    if (other._data)
    {
        this->_data = static_cast<char*>(cma_calloc(this->_capacity + 1, sizeof(char)));
        if (!this->_data)
        {
            this->set_error(FT_ERR_NO_MEMORY);
            return ;
        }
        ft_memcpy(this->_data, other._data, this->_length + 1);
    }
    return ;
}

ft_string::ft_string(ft_string&& other) noexcept
    : _data(other._data),
      _length(other._length),
      _capacity(other._capacity),
      _error_code(other._error_code)
{
    int other_error_code;

    other_error_code = other._error_code;
    this->set_error(other_error_code);
    other._data = ft_nullptr;
    other._length = 0;
    other._capacity = 0;
    other._error_code = ER_SUCCESS;
    return ;
}

ft_string& ft_string::operator=(const ft_string& other) noexcept
{
    if (this == &other)
    {
        this->set_error(other._error_code);
        return (*this);
    }
    cma_free(this->_data);
    this->_data = ft_nullptr;
    this->_length = other._length;
    this->_capacity = other._capacity;
    this->set_error(other._error_code);
    if (other._error_code != ER_SUCCESS)
    {
        this->_length = 0;
        this->_capacity = 0;
        return (*this);
    }
    if (other._data)
    {
        this->_data = static_cast<char*>(cma_calloc(this->_capacity + 1, sizeof(char)));
        if (!this->_data)
        {
            this->set_error(FT_ERR_NO_MEMORY);
            return (*this);
        }
        ft_memcpy(this->_data, other._data, this->_length + 1);
    }
    return (*this);
}

ft_string& ft_string::operator=(const char* other) noexcept
{
    cma_free(this->_data);
    this->_data = ft_nullptr;
    this->_length = 0;
    this->_capacity = 0;
    this->set_error(ER_SUCCESS);
    if (other)
    {
        this->_length = ft_strlen_size_t(other);
        this->_capacity = this->_length;
        this->_data = static_cast<char*>(cma_calloc(this->_capacity + 1, sizeof(char)));
        if (!this->_data)
        {
            this->set_error(FT_ERR_NO_MEMORY);
            return (*this);
        }
        ft_memcpy(this->_data, other, this->_length + 1);
    }
    return (*this);
}

ft_string& ft_string::operator=(ft_string&& other) noexcept
{
    if (this != &other)
    {
        int other_error_code;

        if (this->_error_code != ER_SUCCESS)
        {
            return (*this);
        }
        cma_free(this->_data);
        other_error_code = other._error_code;
        this->_data = ft_nullptr;
        this->_length = 0;
        this->_capacity = 0;
        if (other_error_code != ER_SUCCESS)
        {
            this->set_error(other_error_code);
            other._data = ft_nullptr;
            other._length = 0;
            other._capacity = 0;
            other._error_code = ER_SUCCESS;
            return (*this);
        }
        this->_data = other._data;
        this->_length = other._length;
        this->_capacity = other._capacity;
        this->set_error(other_error_code);
        other._data = ft_nullptr;
        other._length = 0;
        other._capacity = 0;
        other._error_code = ER_SUCCESS;
    }
    return (*this);
}

ft_string::~ft_string()
{
    cma_free(this->_data);
    return ;
}

void *ft_string::operator new(size_t size) noexcept
{
    void* ptr = cma_malloc(size);
    if (!ptr)
        return (ft_nullptr);
    return (ptr);
}

void ft_string::operator delete(void* ptr) noexcept
{
    cma_free(ptr);
    return ;
}

void *ft_string::operator new[](size_t size) noexcept
{
    void* ptr = cma_malloc(size);
    if (!ptr)
        return (ft_nullptr);
    return (ptr);
}

void ft_string::operator delete[](void* ptr) noexcept
{
    cma_free(ptr);
    return ;
}

ft_string::ft_string(int error_code) noexcept
    : _data(ft_nullptr)
    , _length(0)
    , _capacity(0)
    , _error_code(error_code)
{
    this->set_error(error_code);
    return ;
}
