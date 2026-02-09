#include "class_string.hpp"
#include "../CMA/CMA.hpp"
#include "../Basic/basic.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/pthread_internal.hpp"
#include "class_nullptr.hpp"
#include <climits>

void ft_string::sleep_backoff() noexcept
{
    pt_thread_sleep(1);
    return ;
}

int ft_string::enable_thread_safety() noexcept
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESSS);
    pt_recursive_mutex *mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    int mutex_error = mutex_pointer->initialize();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        delete mutex_pointer;
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESSS);
}

int ft_string::disable_thread_safety() noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    int destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_string::is_thread_safe_enabled() const noexcept
{
    return (this->_mutex != ft_nullptr);
}


#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_string::get_mutex_for_validation() const noexcept
{
    return (&(this->_mutex));
}
#endif

int ft_string::resize_buffer(size_t new_capacity) noexcept
{
    if (new_capacity <= this->_capacity)
        return (FT_ERR_SUCCESSS);
    char* new_data = static_cast<char*>(cma_realloc(this->_data, new_capacity + 1));
    if (!new_data)
        return (FT_ERR_SYSTEM);
    this->_data = new_data;
    this->_capacity = new_capacity;
    return (FT_ERR_SUCCESSS);
}

int ft_string::append_char_buffer(char character) noexcept
{
    if (!this->_data)
    {
        int buffer_error = this->ensure_empty_buffer();
        if (buffer_error != FT_ERR_SUCCESSS)
            return (buffer_error);
    }
    if (this->_length + 1 >= this->_capacity)
    {
        size_t new_capacity = this->_capacity;

        if (new_capacity == 0)
            new_capacity = 16;
        else
            new_capacity *= 2;
        int resize_error = this->resize_buffer(new_capacity);
        if (resize_error != FT_ERR_SUCCESSS)
            return (resize_error);
    }
    this->_data[this->_length++] = character;
    this->_data[this->_length] = '\0';
    return (FT_ERR_SUCCESSS);
}

int ft_string::append_buffer(const char *string, size_t length) noexcept
{
    size_t new_capacity;
    size_t index;

    if (!string || length == 0)
        return (FT_ERR_SUCCESSS);
    if (!this->_data)
    {
        int buffer_error = this->ensure_empty_buffer();
        if (buffer_error != FT_ERR_SUCCESSS)
            return (buffer_error);
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
        int resize_error = this->resize_buffer(new_capacity);
        if (resize_error != FT_ERR_SUCCESSS)
            return (resize_error);
    }
    index = 0;
    while (index < length)
    {
        this->_data[this->_length + index] = string[index];
        index++;
    }
    this->_length += length;
    this->_data[this->_length] = '\0';
    return (FT_ERR_SUCCESSS);
}

int ft_string::clear_buffer() noexcept
{
    this->_length = 0;
    if (this->_data)
        this->_data[0] = '\0';
    return (FT_ERR_SUCCESSS);
}

int ft_string::ensure_empty_buffer() noexcept
{
    char *new_data;

    if (this->_data)
    {
        this->_data[0] = '\0';
        if (this->_capacity == 0)
            this->_capacity = 1;
        this->_length = 0;
        return (FT_ERR_SUCCESSS);
    }
    new_data = static_cast<char*>(cma_calloc(1, sizeof(char)));
    if (!new_data)
    {
        return (FT_ERR_SYSTEM);
    }
    this->_data = new_data;
    this->_capacity = 1;
    this->_length = 0;
    this->_data[0] = '\0';
    return (FT_ERR_SUCCESSS);
}

int ft_string::assign_buffer(size_t count, char character) noexcept
{
    size_t index;

    int clear_error = this->clear_buffer();
    if (clear_error != FT_ERR_SUCCESSS)
        return (clear_error);
    if (count == 0)
        return (this->ensure_empty_buffer());
    int resize_error = this->resize_length_buffer(count);
    if (resize_error != FT_ERR_SUCCESSS)
        return (resize_error);
    index = 0;
    while (index < count)
    {
        this->_data[index] = character;
        index++;
    }
    this->_length = count;
    this->_data[this->_length] = '\0';
    return (FT_ERR_SUCCESSS);
}

int ft_string::assign_buffer(const char *string, size_t length) noexcept
{
    if (length == 0)
    {
        int clear_error = this->clear_buffer();
        if (clear_error != FT_ERR_SUCCESSS)
            return (clear_error);
        return (this->ensure_empty_buffer());
    }
    if (!string)
        return (FT_ERR_INVALID_ARGUMENT);
    int clear_error = this->clear_buffer();
    if (clear_error != FT_ERR_SUCCESSS)
        return (clear_error);
    if (this->_capacity < length)
    {
        int resize_error = this->resize_buffer(length);
        if (resize_error != FT_ERR_SUCCESSS)
            return (resize_error);
    }
    return (this->append_buffer(string, length));
}

int ft_string::erase_buffer(std::size_t index, std::size_t count) noexcept
{
    if (index >= this->_length)
        return (FT_ERR_OUT_OF_RANGE);
    if (index + count > this->_length)
        count = this->_length - index;
    if (count > 0)
    {
        ft_memmove(this->_data + index, this->_data + index + count,
                this->_length - index - count);
        this->_length -= count;
        this->_data[this->_length] = '\0';
    }
    return (FT_ERR_SUCCESSS);
}

int ft_string::resize_length_buffer(size_t new_length) noexcept
{
    size_t new_capacity;

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
        if (this->resize_buffer(new_capacity) != FT_ERR_SUCCESSS)
            return (FT_ERR_SYSTEM);
    }
    if (!this->_data && new_length > 0)
    {
        this->_data = static_cast<char*>(cma_calloc(new_length + 1, sizeof(char)));
        if (!this->_data)
            return (FT_ERR_SYSTEM);
        this->_capacity = new_length;
    }
    if (!this->_data)
    {
        this->_length = 0;
        return (FT_ERR_SUCCESSS);
    }
    if (new_length > this->_length)
    {
        size_t index;

        index = this->_length;
        while (index < new_length)
        {
            this->_data[index] = '\0';
            index++;
        }
    }
    this->_length = new_length;
    this->_data[this->_length] = '\0';
    return (FT_ERR_SUCCESSS);
}

int ft_string::move_buffer(ft_string &other) noexcept
{
    if (this == &other)
        return (FT_ERR_SUCCESSS);
    cma_free(this->_data);
    this->_data = other._data;
    this->_length = other._length;
    this->_capacity = other._capacity;
    if (!this->_data)
        this->_capacity = 0;
    other._data = ft_nullptr;
    other._length = 0;
    other._capacity = 0;
    return (FT_ERR_SUCCESSS);
}

int ft_string::resize(size_t new_capacity) noexcept
{
    if (!this->_initialized)
        return (FT_ERR_INVALID_STATE);
    if (this->_mutex != ft_nullptr)
    {
        int lock_error = this->_mutex->lock();
        if (lock_error != FT_ERR_SUCCESSS)
            return (lock_error);
        int result = this->resize_buffer(new_capacity);
        this->_mutex->unlock();
        return (result);
    }
    return (this->resize_buffer(new_capacity));
}

int ft_string::append(char character) noexcept
{
    if (!this->_initialized)
        return (FT_ERR_INVALID_STATE);
    if (this->_mutex != ft_nullptr)
    {
        int lock_error = this->_mutex->lock();
        if (lock_error != FT_ERR_SUCCESSS)
            return (lock_error);
        int result = this->append_char_buffer(character);
        this->_mutex->unlock();
        return (result);
    }
    return (this->append_char_buffer(character));
}

int ft_string::append(const ft_string& string) noexcept
{
    if (!this->_initialized || !string._initialized)
        return (FT_ERR_INVALID_STATE);
    if (string._length == 0)
        return (FT_ERR_SUCCESSS);
    pt_recursive_mutex *self_mutex = this->_mutex;
    pt_recursive_mutex *other_mutex = string._mutex;
    bool self_locked = false;
    bool other_locked = false;
    if (self_mutex != ft_nullptr && other_mutex != ft_nullptr)
    {
        if (self_mutex == other_mutex)
        {
            int lock_error = self_mutex->lock();
            if (lock_error != FT_ERR_SUCCESSS)
                return (lock_error);
            int result = this->append_buffer(string._data, string._length);
            self_mutex->unlock();
            return (result);
        }
        pt_recursive_mutex *first = self_mutex;
        pt_recursive_mutex *second = other_mutex;
        bool first_is_self = true;
        bool second_is_self = false;
        if (first > second)
        {
            pt_recursive_mutex *temporary = first;
            first = second;
            second = temporary;
            first_is_self = false;
            second_is_self = true;
        }
        int lock_error = first->lock();
        if (lock_error != FT_ERR_SUCCESSS)
            return (lock_error);
        if (first_is_self)
            self_locked = true;
        else
            other_locked = true;
        lock_error = second->lock();
        if (lock_error != FT_ERR_SUCCESSS)
        {
            if (second_is_self)
                self_locked = true;
            else
                other_locked = true;
            if (self_locked)
                self_mutex->unlock();
            if (other_locked && other_mutex != self_mutex)
                other_mutex->unlock();
            return (lock_error);
        }
        if (second_is_self)
            self_locked = true;
        else
            other_locked = true;
    }
    else if (self_mutex != ft_nullptr)
    {
        int lock_error = self_mutex->lock();
        if (lock_error != FT_ERR_SUCCESSS)
            return (lock_error);
        self_locked = true;
    }
    else if (other_mutex != ft_nullptr)
    {
        int lock_error = other_mutex->lock();
        if (lock_error != FT_ERR_SUCCESSS)
            return (lock_error);
        other_locked = true;
    }
    int result = this->append_buffer(string._data, string._length);
    if (other_locked && other_mutex != self_mutex)
        other_mutex->unlock();
    if (self_locked)
        self_mutex->unlock();
    return (result);
}

int ft_string::append(const char *string) noexcept
{
    if (!this->_initialized)
        return (FT_ERR_INVALID_STATE);
    if (string == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    size_t string_length = 0;
    while (string[string_length])
        string_length++;
    return (this->append(string, string_length));
}

int ft_string::clear() noexcept
{
    if (!this->_initialized)
        return (FT_ERR_INVALID_STATE);
    if (this->_mutex != ft_nullptr)
    {
        int lock_error = this->_mutex->lock();
        if (lock_error != FT_ERR_SUCCESSS)
            return (lock_error);
        int result = this->clear_buffer();
        this->_mutex->unlock();
        return (result);
    }
    return (this->clear_buffer());
}

const char* ft_string::at(size_t index) const noexcept
{
    if (!this->_initialized)
        return (ft_nullptr);
    if (this->_mutex != ft_nullptr)
    {
        int lock_error = this->_mutex->lock();
        if (lock_error != FT_ERR_SUCCESSS)
            return (ft_nullptr);
        if (index >= this->_length)
        {
            this->_mutex->unlock();
            return (ft_nullptr);
        }
        const char *value = &this->_data[index];
        this->_mutex->unlock();
        return (value);
    }
    if (index >= this->_length)
        return (ft_nullptr);
    return (&this->_data[index]);
}

const char* ft_string::c_str() const noexcept
{
    if (!this->_initialized)
        return (const_cast<char *>(""));
    if (this->_mutex != ft_nullptr)
    {
        int lock_error = this->_mutex->lock();
        if (lock_error != FT_ERR_SUCCESSS)
            return (const_cast<char *>(""));
        const char *value = (this->_data ? this->_data : const_cast<char *>(""));
        this->_mutex->unlock();
        return (value);
    }
    if (this->_data)
        return (this->_data);
    return (const_cast<char *>(""));
}

char* ft_string::data() noexcept
{
    if (!this->_initialized)
        return (ft_nullptr);
    if (this->_mutex != ft_nullptr)
    {
        int lock_error = this->_mutex->lock();
        if (lock_error != FT_ERR_SUCCESSS)
            return (ft_nullptr);
        char *value = this->_data;
        this->_mutex->unlock();
        return (value);
    }
    return (this->_data);
}

const char* ft_string::data() const noexcept
{
    return (this->c_str());
}

char* ft_string::print() noexcept
{
    return (this->data());
}

size_t ft_string::size() const noexcept
{
    if (!this->_initialized)
        return (0);
    if (this->_mutex != ft_nullptr)
    {
        int lock_error = this->_mutex->lock();
        if (lock_error != FT_ERR_SUCCESSS)
            return (0);
        size_t length = this->_length;
        this->_mutex->unlock();
        return (length);
    }
    return (this->_length);
}

bool ft_string::empty() const noexcept
{
    return (this->size() == 0);
}

int ft_string::move(ft_string& other) noexcept
{
    if (!this->_initialized || !other._initialized)
        return (FT_ERR_INVALID_STATE);
    pt_recursive_mutex *self_mutex = this->_mutex;
    pt_recursive_mutex *other_mutex = other._mutex;
    bool self_locked = false;
    bool other_locked = false;
    if (self_mutex != ft_nullptr && other_mutex != ft_nullptr)
    {
        pt_recursive_mutex *first = self_mutex;
        pt_recursive_mutex *second = other_mutex;
        bool first_is_self = true;
        bool second_is_self = false;
        if (first > second)
        {
            pt_recursive_mutex *temporary = first;
            first = second;
            second = temporary;
            first_is_self = false;
            second_is_self = true;
        }
        int lock_error = first->lock();
        if (lock_error != FT_ERR_SUCCESSS)
            return (lock_error);
        if (first_is_self)
            self_locked = true;
        else
            other_locked = true;
        lock_error = second->lock();
        if (lock_error != FT_ERR_SUCCESSS)
        {
            if (second_is_self)
                self_locked = true;
            else
                other_locked = true;
            if (self_locked)
                self_mutex->unlock();
            if (other_locked && other_mutex != self_mutex)
                other_mutex->unlock();
            return (lock_error);
        }
        if (second_is_self)
            self_locked = true;
        else
            other_locked = true;
    }
    else if (self_mutex != ft_nullptr)
    {
        int lock_error = self_mutex->lock();
        if (lock_error != FT_ERR_SUCCESSS)
            return (lock_error);
        self_locked = true;
    }
    else if (other_mutex != ft_nullptr)
    {
        int lock_error = other_mutex->lock();
        if (lock_error != FT_ERR_SUCCESSS)
            return (lock_error);
        other_locked = true;
    }
    int result = this->move_buffer(other);
    if (other_locked && other_mutex != self_mutex)
        other_mutex->unlock();
    if (self_locked)
        self_mutex->unlock();
    return (result);
}

int ft_string::erase(std::size_t index, std::size_t count) noexcept
{
    if (!this->_initialized)
        return (FT_ERR_INVALID_STATE);
    if (this->_mutex != ft_nullptr)
    {
        int lock_error = this->_mutex->lock();
        if (lock_error != FT_ERR_SUCCESSS)
            return (lock_error);
        int result = this->erase_buffer(index, count);
        this->_mutex->unlock();
        return (result);
    }
    return (this->erase_buffer(index, count));
}

int ft_string::append(const char *string, size_t length) noexcept
{
    if (!this->_initialized)
        return (FT_ERR_INVALID_STATE);
    if (!string)
        return (FT_ERR_INVALID_ARGUMENT);
    if (length == 0)
        return (FT_ERR_SUCCESSS);
    if (this->_mutex != ft_nullptr)
    {
        int lock_error = this->_mutex->lock();
        if (lock_error != FT_ERR_SUCCESSS)
            return (lock_error);
        int result = this->append_buffer(string, length);
        this->_mutex->unlock();
        return (result);
    }
    return (this->append_buffer(string, length));
}

int ft_string::assign(size_t count, char character) noexcept
{
    if (!this->_initialized)
        return (FT_ERR_INVALID_STATE);
    if (this->_mutex != ft_nullptr)
    {
        int lock_error = this->_mutex->lock();
        if (lock_error != FT_ERR_SUCCESSS)
            return (lock_error);
        int result = this->assign_buffer(count, character);
        this->_mutex->unlock();
        return (result);
    }
    return (this->assign_buffer(count, character));
}

int ft_string::assign(const char *string, size_t length) noexcept
{
    if (!this->_initialized)
        return (FT_ERR_INVALID_STATE);
    if (this->_mutex != ft_nullptr)
    {
        int lock_error = this->_mutex->lock();
        if (lock_error != FT_ERR_SUCCESSS)
            return (lock_error);
        int result = this->assign_buffer(string, length);
        this->_mutex->unlock();
        return (result);
    }
    return (this->assign_buffer(string, length));
}

int ft_string::resize_length(size_t new_length) noexcept
{
    if (!this->_initialized)
        return (FT_ERR_INVALID_STATE);
    if (this->_mutex != ft_nullptr)
    {
        int lock_error = this->_mutex->lock();
        if (lock_error != FT_ERR_SUCCESSS)
            return (lock_error);
        int result = this->resize_length_buffer(new_length);
        this->_mutex->unlock();
        return (result);
    }
    return (this->resize_length_buffer(new_length));
}

int ft_string::push_back(char character) noexcept
{
    return (this->append(character));
}

char ft_string::back() noexcept
{
    if (!this->_initialized)
        return ('\0');
    if (this->_mutex != ft_nullptr)
    {
        int lock_error = this->_mutex->lock();
        if (lock_error != FT_ERR_SUCCESSS)
            return ('\0');
        if (this->_length == 0)
        {
            this->_mutex->unlock();
            return ('\0');
        }
        char result = this->_data[this->_length - 1];
        this->_mutex->unlock();
        return (result);
    }
    if (this->_length == 0)
        return ('\0');
    return (this->_data[this->_length - 1]);
}

size_t ft_string::find(const char *substring) const noexcept
{
    if (!this->_initialized || !substring)
        return (ft_string::npos);
    auto search_without_lock = [this, substring]() -> size_t {
        size_t substring_length = 0;
        while (substring[substring_length])
            substring_length++;
        if (substring_length == 0)
            return (0);
        if (substring_length > this->_length)
            return (ft_string::npos);
        size_t index = 0;
        while (index + substring_length <= this->_length)
        {
            size_t match_index = 0;
            while (match_index < substring_length
                    && this->_data[index + match_index] == substring[match_index])
                match_index++;
            if (match_index == substring_length)
                return (index);
            index++;
        }
        return (ft_string::npos);
    };
    if (this->_mutex != ft_nullptr)
    {
        int lock_error = this->_mutex->lock();
        if (lock_error != FT_ERR_SUCCESSS)
            return (ft_string::npos);
        size_t result = search_without_lock();
        this->_mutex->unlock();
        return (result);
    }
    return (search_without_lock());
}

size_t ft_string::find(const ft_string &substring) const noexcept
{
    if (!this->_initialized || !substring._initialized)
        return (ft_string::npos);
    if (substring._length == 0)
        return (0);
    if (substring._length > this->_length)
        return (ft_string::npos);
    pt_recursive_mutex *self_mutex = this->_mutex;
    pt_recursive_mutex *other_mutex = substring._mutex;
    auto search_without_lock = [this, &substring]() -> size_t {
        size_t index = 0;
        while (index + substring._length <= this->_length)
        {
            size_t match_index = 0;
            while (match_index < substring._length
                    && this->_data[index + match_index] == substring._data[match_index])
                match_index++;
            if (match_index == substring._length)
                return (index);
            index++;
        }
        return (ft_string::npos);
    };
    bool self_locked = false;
    bool other_locked = false;
    if (self_mutex != ft_nullptr && other_mutex != ft_nullptr)
    {
        pt_recursive_mutex *first = self_mutex;
        pt_recursive_mutex *second = other_mutex;
        bool first_is_self = true;
        bool second_is_self = false;
        if (first > second)
        {
            pt_recursive_mutex *temporary = first;
            first = second;
            second = temporary;
            first_is_self = false;
            second_is_self = true;
        }
        int lock_error = first->lock();
        if (lock_error != FT_ERR_SUCCESSS)
            return (ft_string::npos);
        if (first_is_self)
            self_locked = true;
        else
            other_locked = true;
        lock_error = second->lock();
        if (lock_error != FT_ERR_SUCCESSS)
        {
            if (second_is_self)
                self_locked = true;
            else
                other_locked = true;
            if (self_locked)
                self_mutex->unlock();
            if (other_locked && other_mutex != self_mutex)
                other_mutex->unlock();
            return (ft_string::npos);
        }
        if (second_is_self)
            self_locked = true;
        else
            other_locked = true;
    }
    else if (self_mutex != ft_nullptr)
    {
        int lock_error = self_mutex->lock();
        if (lock_error != FT_ERR_SUCCESSS)
            return (ft_string::npos);
        self_locked = true;
    }
    else if (other_mutex != ft_nullptr)
    {
        int lock_error = other_mutex->lock();
        if (lock_error != FT_ERR_SUCCESSS)
            return (ft_string::npos);
        other_locked = true;
    }
    size_t result = search_without_lock();
    if (other_locked && other_mutex != self_mutex)
        other_mutex->unlock();
    if (self_locked)
        self_mutex->unlock();
    return (result);
}

ft_string ft_string::substr(size_t index, size_t count) const noexcept
{
    ft_string substring;
    if (!this->_initialized)
        return (substring);
    if (substring.initialize() != FT_ERR_SUCCESSS)
        return (substring);
    if (this->_mutex != ft_nullptr)
    {
        int lock_error = this->_mutex->lock();
        if (lock_error != FT_ERR_SUCCESSS)
            return (substring);
    }
    if (index > this->_length)
    {
        if (this->_mutex != ft_nullptr)
            this->_mutex->unlock();
        return (substring);
    }
    size_t available_length = this->_length - index;
    size_t copy_length = count;
    if (copy_length == ft_string::npos || copy_length > available_length)
        copy_length = available_length;
    if (copy_length > 0)
    {
        if (substring.assign(this->_data + index, copy_length) != FT_ERR_SUCCESSS)
        {
            if (this->_mutex != ft_nullptr)
                this->_mutex->unlock();
            substring.destroy();
            return (substring);
        }
    }
    if (this->_mutex != ft_nullptr)
        this->_mutex->unlock();
    return (substring);
}


ft_string::operator const char*() const noexcept
{
    return (this->c_str());
}
