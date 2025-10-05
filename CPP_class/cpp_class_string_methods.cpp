#include "class_string_class.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include "class_nullptr.hpp"
#include <climits>

void ft_string::resize(size_t new_capacity) noexcept
{
    if (new_capacity <= this->_capacity)
    {
        this->set_error(ER_SUCCESS);
        return ;
    }
    char* new_data = static_cast<char*>(cma_realloc(this->_data, new_capacity + 1));
    if (!new_data)
    {
        this->set_error(STRING_MEM_ALLOC_FAIL);
        return ;
    }
    this->_data = new_data;
    this->_capacity = new_capacity;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_string::append(char c) noexcept
{
    if (this->_error_code != ER_SUCCESS)
    {
        return ;
    }
    if (this->_length + 1 >= this->_capacity)
    {
        size_t new_capacity = this->_capacity;
        if (new_capacity == 0)
            new_capacity = 16;
        else
            new_capacity *= 2;
        this->resize(new_capacity);
        if (this->_error_code)
            return ;
    }
    this->_data[this->_length++] = c;
    this->_data[this->_length] = '\0';
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_string::append(const ft_string& string) noexcept
{
    if (this->_error_code != ER_SUCCESS)
    {
        return ;
    }
    if (string._error_code != ER_SUCCESS)
    {
        this->set_error(string._error_code);
        return ;
    }
    if (string._length == 0)
    {
        this->set_error(ER_SUCCESS);
        return ;
    }
    size_t new_length = this->_length + string._length;
    if (new_length >= this->_capacity)
    {
        size_t new_capacity = this->_capacity;
        if (new_capacity == 0)
            new_capacity = 16;
        while (new_capacity <= new_length)
            new_capacity *= 2;
        this->resize(new_capacity);
        if (this->_error_code)
            return ;
    }
    ft_memcpy(this->_data + this->_length, string._data, string._length);
    this->_length = new_length;
    this->_data[this->_length] = '\0';
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_string::append(const char *string) noexcept
{
    size_t string_length;

    if (this->_error_code != ER_SUCCESS)
    {
        return ;
    }
    if (!string)
    {
        this->set_error(ER_SUCCESS);
        return ;
    }
    string_length = ft_strlen_size_t(string);
    this->append(string, string_length);
    return ;
}

void ft_string::clear() noexcept
{
    if (this->_error_code != ER_SUCCESS)
    {
        return ;
    }
    this->_length = 0;
    if (this->_data)
        this->_data[0] = '\0';
    this->set_error(ER_SUCCESS);
    return ;
}

const char* ft_string::at(size_t index) const noexcept
{
    if (index >= this->_length)
        return (ft_nullptr);
    return (&this->_data[index]);
}

const char* ft_string::c_str() const noexcept
{
    if (this->_data)
        return (this->_data);
    return (const_cast<char *>(""));
}

char* ft_string::data() noexcept
{
    return (this->_data);
}

const char* ft_string::data() const noexcept
{
    return (this->c_str());
}

char* ft_string::print() noexcept
{
    if (this->_data)
        return (this->_data);
    return (const_cast<char *>(""));
}

size_t ft_string::size() const noexcept
{
    return (this->_length);
}

bool ft_string::empty() const noexcept
{
    return (this->_length == 0);
}

int ft_string::get_error() const noexcept
{
    return (this->_error_code);
}

const char* ft_string::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

void ft_string::set_error(int error_code) noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

void ft_string::move(ft_string& other) noexcept
{
    if (this != &other)
    {
        int other_error_code;

        if (this->_error_code != ER_SUCCESS)
        {
            return ;
        }
        other_error_code = other._error_code;
        cma_free(this->_data);
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
            return ;
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
    return ;
}

ft_string& ft_string::operator+=(const ft_string& other) noexcept
{
    if (this->_error_code != ER_SUCCESS)
    {
        return (*this);
    }
    if (other._error_code != ER_SUCCESS)
    {
        this->set_error(other._error_code);
        return (*this);
    }
    this->append(other);
    return (*this);
}

ft_string& ft_string::operator+=(const char* cstr) noexcept
{
    if (this->_error_code != ER_SUCCESS)
    {
        return (*this);
    }
    if (cstr)
    {
        size_t i = 0;
        while (cstr[i] != '\0')
        {
            this->append(cstr[i]);
            if (this->_error_code)
                return (*this);
            ++i;
        }
    }
    return (*this);
}

ft_string& ft_string::operator+=(char c) noexcept
{
    this->append(c);
    return (*this);
}

void ft_string::erase(std::size_t index, std::size_t count) noexcept
{
    if (this->_error_code != ER_SUCCESS)
    {
        return ;
    }
    if (index >= this->_length)
    {
        this->set_error(STRING_ERASE_OUT_OF_BOUNDS);
        return ;
    }
    if (index + count > this->_length)
        count = this->_length - index;
    if (count > 0)
    {
        ft_memmove(this->_data + index, this->_data + index + count,
                this->_length - index - count);
        this->_length -= count;
        this->_data[this->_length] = '\0';
    }
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_string::append(const char *string, size_t length) noexcept
{
    size_t new_capacity;
    size_t index;

    if (this->_error_code != ER_SUCCESS)
    {
        return ;
    }
    if (!string || length == 0)
    {
        this->set_error(ER_SUCCESS);
        return ;
    }
    if (this->_length + length >= this->_capacity)
    {
        new_capacity = this->_capacity;
        if (new_capacity == 0)
            new_capacity = 16;
        while (this->_length + length >= new_capacity)
        {
            if (new_capacity > SIZE_MAX / 2)
            {
                new_capacity = this->_length + length + 1;
                break ;
            }
            new_capacity *= 2;
        }
        this->resize(new_capacity);
        if (this->_error_code)
            return ;
    }
    index = 0;
    while (index < length)
    {
        this->_data[this->_length + index] = string[index];
        index++;
    }
    this->_length += length;
    this->_data[this->_length] = '\0';
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_string::assign(size_t count, char character) noexcept
{
    size_t index;

    if (this->_error_code != ER_SUCCESS)
    {
        return ;
    }
    this->clear();
    if (this->_error_code != ER_SUCCESS)
    {
        return ;
    }
    if (count == 0)
    {
        this->set_error(ER_SUCCESS);
        return ;
    }
    this->resize_length(count);
    if (this->_error_code)
        return ;
    index = 0;
    while (index < count)
    {
        this->_data[index] = character;
        index++;
    }
    this->_length = count;
    this->_data[this->_length] = '\0';
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_string::assign(const char *string, size_t length) noexcept
{
    if (this->_error_code != ER_SUCCESS)
    {
        return ;
    }
    this->clear();
    if (this->_error_code != ER_SUCCESS)
    {
        return ;
    }
    if (!string || length == 0)
        return ;
    if (this->_capacity < length)
    {
        this->resize(length);
        if (this->_error_code)
            return ;
    }
    this->append(string, length);
    return ;
}

void ft_string::resize_length(size_t new_length) noexcept
{
    size_t new_capacity;

    if (this->_error_code != ER_SUCCESS)
    {
        return ;
    }
    if (new_length >= this->_capacity)
    {
        new_capacity = this->_capacity;
        if (new_capacity == 0)
            new_capacity = 16;
        while (new_capacity <= new_length)
        {
            if (new_capacity > SIZE_MAX / 2)
            {
                new_capacity = new_length + 1;
                break ;
            }
            new_capacity *= 2;
        }
        this->resize(new_capacity);
        if (this->_error_code)
            return ;
    }
    if (!this->_data && new_length > 0)
    {
        this->_data = static_cast<char*>(cma_calloc(new_length + 1, sizeof(char)));
        if (!this->_data)
        {
            this->set_error(STRING_MEM_ALLOC_FAIL);
            return ;
        }
        this->_capacity = new_length;
    }
    if (!this->_data)
    {
        this->_length = 0;
        this->set_error(ER_SUCCESS);
        return ;
    }
    if (new_length > this->_length)
    {
        size_t index = this->_length;
        while (index < new_length)
        {
            this->_data[index] = '\0';
            index++;
        }
    }
    this->_length = new_length;
    this->_data[this->_length] = '\0';
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_string::push_back(char character) noexcept
{
    if (this->_error_code != ER_SUCCESS)
    {
        return ;
    }
    this->append(character);
    return ;
}

char ft_string::back() noexcept
{
    if (this->_error_code != ER_SUCCESS)
    {
        return ('\0');
    }
    if (this->_length == 0)
    {
        this->set_error(STRING_ERASE_OUT_OF_BOUNDS);
        return ('\0');
    }
    this->set_error(ER_SUCCESS);
    return (this->_data[this->_length - 1]);
}

size_t ft_string::find(const char *substring) const noexcept
{
    size_t substring_length;
    size_t index;
    size_t match_index;

    if (this->_error_code != ER_SUCCESS)
    {
        return (ft_string::npos);
    }
    if (!substring)
        return (ft_string::npos);
    substring_length = ft_strlen_size_t(substring);
    if (substring_length == 0)
        return (0);
    if (substring_length > this->_length)
        return (ft_string::npos);
    index = 0;
    while (index + substring_length <= this->_length)
    {
        match_index = 0;
        while (match_index < substring_length
            && this->_data[index + match_index] == substring[match_index])
        {
            match_index++;
        }
        if (match_index == substring_length)
            return (index);
        index++;
    }
    return (ft_string::npos);
}

size_t ft_string::find(const ft_string &substring) const noexcept
{
    if (this->_error_code != ER_SUCCESS)
    {
        return (ft_string::npos);
    }
    if (substring._error_code != ER_SUCCESS)
    {
        return (ft_string::npos);
    }
    if (substring._length == 0)
        return (0);
    return (this->find(substring.c_str()));
}

ft_string ft_string::substr(size_t index, size_t count) const noexcept
{
    ft_string substring;
    size_t available_length;
    size_t copy_length;

    if (this->_error_code != ER_SUCCESS)
    {
        substring.set_error(this->_error_code);
        return (substring);
    }
    if (index > this->_length)
    {
        substring.set_error(STRING_ERASE_OUT_OF_BOUNDS);
        return (substring);
    }
    available_length = this->_length - index;
    copy_length = count;
    if (copy_length == ft_string::npos || copy_length > available_length)
        copy_length = available_length;
    if (copy_length == 0)
        return (substring);
    substring.assign(this->_data + index, copy_length);
    return (substring);
}

ft_string operator+(const ft_string &lhs, const ft_string &rhs) noexcept
{
    ft_string result(lhs);
    result += rhs;
    return (result);
}

ft_string operator+(const ft_string &lhs, const char *rhs) noexcept
{
    ft_string result(lhs);
    result += rhs;
    return (result);
}

ft_string operator+(const char *lhs, const ft_string &rhs) noexcept
{
    ft_string result;
    if (lhs)
        result = lhs;
    else
        result = "";
    result += rhs;
    return (result);
}

ft_string operator+(const ft_string &lhs, char rhs) noexcept
{
    ft_string result(lhs);
    result += rhs;
    return (result);
}

ft_string operator+(char lhs, const ft_string &rhs) noexcept
{
    ft_string result;
    result += lhs;
    result += rhs;
    return (result);
}

ft_string::operator const char*() const noexcept
{
    return (this->c_str());
}

bool operator==(const ft_string &lhs, const ft_string &rhs) noexcept
{
    return (ft_strcmp(lhs.c_str(), rhs.c_str()) == 0);
}

bool operator==(const ft_string &lhs, const char* rhs) noexcept
{
    return (ft_strcmp(lhs.c_str(), rhs) == 0);
}

bool operator==(const char* lhs, const ft_string &rhs) noexcept
{
    return (ft_strcmp(lhs, rhs.c_str()) == 0);
}

bool operator<(const ft_string &lhs, const ft_string &rhs) noexcept
{
    bool result;

    result = ft_strcmp(lhs.c_str(), rhs.c_str()) < 0;
    return (result);
}

bool operator<(const ft_string &lhs, const char* rhs) noexcept
{
    bool result;

    result = ft_strcmp(lhs.c_str(), rhs) < 0;
    return (result);
}

bool operator<(const char* lhs, const ft_string &rhs) noexcept
{
    bool result;

    result = ft_strcmp(lhs, rhs.c_str()) < 0;
    return (result);
}

bool operator>(const ft_string &lhs, const ft_string &rhs) noexcept
{
    bool result;

    result = ft_strcmp(lhs.c_str(), rhs.c_str()) > 0;
    return (result);
}

bool operator>(const ft_string &lhs, const char* rhs) noexcept
{
    bool result;

    result = ft_strcmp(lhs.c_str(), rhs) > 0;
    return (result);
}

bool operator>(const char* lhs, const ft_string &rhs) noexcept
{
    bool result;

    result = ft_strcmp(lhs, rhs.c_str()) > 0;
    return (result);
}
