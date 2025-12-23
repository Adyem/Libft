#include "class_string.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"
#include "class_nullptr.hpp"
#include <climits>

void ft_string::sleep_backoff() noexcept
{
    pt_thread_sleep(1);
    return ;
}

ft_string::mutex_guard::mutex_guard() noexcept
    : _mutex(ft_nullptr), _owns_lock(false), _error_code(FT_ERR_SUCCESSS)
{
    return ;
}

ft_string::mutex_guard::~mutex_guard()
{
    this->unlock();
    return ;
}

ft_string::mutex_guard::mutex_guard(mutex_guard &&other) noexcept
    : _mutex(other._mutex), _owns_lock(other._owns_lock), _error_code(other._error_code)
{
    other._mutex = ft_nullptr;
    other._owns_lock = false;
    other._error_code = FT_ERR_SUCCESSS;
    return ;
}

ft_string::mutex_guard &ft_string::mutex_guard::operator=(mutex_guard &&other) noexcept
{
    if (this != &other)
    {
        this->unlock();
        this->_mutex = other._mutex;
        this->_owns_lock = other._owns_lock;
        this->_error_code = other._error_code;
        other._mutex = ft_nullptr;
        other._owns_lock = false;
        other._error_code = FT_ERR_SUCCESSS;
    }
    return (*this);
}

int ft_string::mutex_guard::lock(pt_recursive_mutex &mutex) noexcept
{
    int lock_error;

    this->unlock();
    lock_error = mutex.lock(THREAD_ID);
    if (lock_error == FT_ERR_SUCCESSS)
    {
        this->_mutex = &mutex;
        this->_owns_lock = true;
        this->_error_code = FT_ERR_SUCCESSS;
    }
    else
    {
        this->_mutex = ft_nullptr;
        this->_owns_lock = false;
        this->_error_code = lock_error;
    }
    return (lock_error);
}

void ft_string::mutex_guard::unlock() noexcept
{
    if (this->_owns_lock && this->_mutex != ft_nullptr)
    {
        this->_mutex->unlock(THREAD_ID);
        this->_owns_lock = false;
    }
    return ;
}

bool ft_string::mutex_guard::owns_lock() const noexcept
{
    return (this->_owns_lock);
}

int ft_string::mutex_guard::get_error() const noexcept
{
    return (this->_error_code);
}

void ft_string::set_error_unlocked(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

void ft_string::set_error(int error_code) const noexcept
{
    this->set_error_unlocked(error_code);
    return ;
}

void ft_string::set_system_error_unlocked(int error_code) const noexcept
{
    this->_system_error_code = error_code;
    ft_sys_errno = error_code;
    return ;
}

void ft_string::set_system_error(int error_code) const noexcept
{
    this->set_system_error_unlocked(error_code);
    return ;
}

int ft_string::lock_self(mutex_guard &guard) const noexcept
{
    int lock_error;

    lock_error = guard.lock(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_system_error(lock_error);
        return (lock_error);
    }
    this->set_system_error(FT_SYS_ERR_SUCCESS);
    return (FT_ERR_SUCCESSS);
}

int ft_string::lock_pair(const ft_string &first, const ft_string &second,
        mutex_guard &first_guard,
        mutex_guard &second_guard) noexcept
{
    const ft_string *ordered_first;
    const ft_string *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        mutex_guard single_guard;

        if (single_guard.lock(first._mutex) != FT_ERR_SUCCESSS)
        {
            first.set_system_error(single_guard.get_error());
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard.unlock();
        first.set_system_error(FT_SYS_ERR_SUCCESS);
        return (FT_ERR_SUCCESSS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_string *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        mutex_guard lower_guard;

        if (lower_guard.lock(ordered_first->_mutex) != FT_ERR_SUCCESSS)
        {
            ordered_first->set_system_error(lower_guard.get_error());
            return (lower_guard.get_error());
        }
        mutex_guard upper_guard;
        if (upper_guard.lock(ordered_second->_mutex) == FT_ERR_SUCCESSS)
        {
            if (!swapped)
            {
                first_guard = ft_move(lower_guard);
                second_guard = ft_move(upper_guard);
            }
            else
            {
                first_guard = ft_move(upper_guard);
                second_guard = ft_move(lower_guard);
            }
            ordered_first->set_system_error(FT_SYS_ERR_SUCCESS);
            ordered_second->set_system_error(FT_SYS_ERR_SUCCESS);
            return (FT_ERR_SUCCESSS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ordered_second->set_system_error(upper_guard.get_error());
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        ft_string::sleep_backoff();
    }
}

void ft_string::resize_unlocked(size_t new_capacity) noexcept
{
    if (new_capacity <= this->_capacity)
    {
        this->set_error_unlocked(FT_ERR_SUCCESSS);
        return ;
    }
    char* new_data = static_cast<char*>(cma_realloc(this->_data, new_capacity + 1));
    if (!new_data)
    {
        this->set_system_error(FT_SYS_ERR_NO_MEMORY);
        this->set_error_unlocked(FT_ERR_SYSTEM);
        return ;
    }
    this->_data = new_data;
    this->_capacity = new_capacity;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

void ft_string::append_char_unlocked(char c) noexcept
{
    if (this->_length + 1 >= this->_capacity)
    {
        size_t new_capacity;

        new_capacity = this->_capacity;
        if (new_capacity == 0)
            new_capacity = 16;
        else
            new_capacity *= 2;
        this->resize_unlocked(new_capacity);
        if (this->_error_code != FT_ERR_SUCCESSS)
            return ;
    }
    this->_data[this->_length++] = c;
    this->_data[this->_length] = '\0';
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

void ft_string::append_unlocked(const char *string, size_t length) noexcept
{
    size_t new_capacity;
    size_t index;

    if (!string || length == 0)
    {
        this->set_error_unlocked(FT_ERR_SUCCESSS);
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
        this->resize_unlocked(new_capacity);
        if (this->_error_code != FT_ERR_SUCCESSS)
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
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

void ft_string::clear_unlocked() noexcept
{
    this->_length = 0;
    if (this->_data)
        this->_data[0] = '\0';
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

void ft_string::ensure_empty_buffer_unlocked() noexcept
{
    char *new_data;

    if (this->_data)
    {
        this->_data[0] = '\0';
        if (this->_capacity == 0)
            this->_capacity = 1;
        this->_length = 0;
        this->set_error_unlocked(FT_ERR_SUCCESSS);
        return ;
    }
    new_data = static_cast<char*>(cma_calloc(1, sizeof(char)));
    if (!new_data)
    {
        this->set_system_error(FT_SYS_ERR_NO_MEMORY);
        return ;
    }
    this->_data = new_data;
    this->_capacity = 1;
    this->_length = 0;
    this->_data[0] = '\0';
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

void ft_string::assign_unlocked(size_t count, char character) noexcept
{
    size_t index;

    this->clear_unlocked();
    if (this->_error_code != FT_ERR_SUCCESSS)
        return ;
    if (count == 0)
    {
        this->ensure_empty_buffer_unlocked();
        return ;
    }
    this->resize_length_unlocked(count);
    if (this->_error_code != FT_ERR_SUCCESSS)
        return ;
    index = 0;
    while (index < count)
    {
        this->_data[index] = character;
        index++;
    }
    this->_length = count;
    this->_data[this->_length] = '\0';
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

void ft_string::assign_unlocked(const char *string, size_t length) noexcept
{
    if (length == 0)
    {
        this->clear_unlocked();
        if (this->_error_code != FT_ERR_SUCCESSS)
            return ;
        this->ensure_empty_buffer_unlocked();
        return ;
    }
    if (!string)
    {
        this->set_error_unlocked(FT_ERR_SUCCESSS);
        return ;
    }
    this->clear_unlocked();
    if (this->_error_code != FT_ERR_SUCCESSS)
        return ;
    if (this->_capacity < length)
    {
        this->resize_unlocked(length);
        if (this->_error_code != FT_ERR_SUCCESSS)
            return ;
    }
    this->append_unlocked(string, length);
    return ;
}

void ft_string::erase_unlocked(std::size_t index, std::size_t count) noexcept
{
    if (index >= this->_length)
    {
        this->set_error_unlocked(FT_ERR_OUT_OF_RANGE);
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
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

void ft_string::resize_length_unlocked(size_t new_length) noexcept
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
        this->resize_unlocked(new_capacity);
        if (this->_error_code != FT_ERR_SUCCESSS)
            return ;
    }
    if (!this->_data && new_length > 0)
    {
        this->_data = static_cast<char*>(cma_calloc(new_length + 1, sizeof(char)));
        if (!this->_data)
        {
            this->set_system_error(FT_SYS_ERR_NO_MEMORY);
            return ;
        }
        this->_capacity = new_length;
    }
    if (!this->_data)
    {
        this->_length = 0;
        this->set_error_unlocked(FT_ERR_SUCCESSS);
        return ;
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
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

void ft_string::move_unlocked(ft_string &other) noexcept
{
    int other_error_code;

    if (this == &other)
        return ;
    other_error_code = other._error_code;
    cma_free(this->_data);
    this->_data = ft_nullptr;
    this->_length = 0;
    this->_capacity = 0;
    if (other_error_code != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(other_error_code);
        this->_system_error_code = other._system_error_code;
        other._data = ft_nullptr;
        other._length = 0;
        other._capacity = 0;
        other._error_code = FT_ERR_SUCCESSS;
        other._system_error_code = FT_SYS_ERR_SUCCESS;
        return ;
    }
    this->_data = other._data;
    this->_length = other._length;
    this->_capacity = other._capacity;
    this->set_error_unlocked(other_error_code);
    this->_system_error_code = other._system_error_code;
    other._data = ft_nullptr;
    other._length = 0;
    other._capacity = 0;
    other._error_code = FT_ERR_SUCCESSS;
    other._system_error_code = FT_SYS_ERR_SUCCESS;
    return ;
}

void ft_string::resize(size_t new_capacity) noexcept
{
    ft_string::mutex_guard guard;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_system_error(lock_error);
        return ;
    }
    this->resize_unlocked(new_capacity);
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void ft_string::append(char c) noexcept
{
    ft_string::mutex_guard guard;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_system_error(lock_error);
        return ;
    }
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(this->_error_code);
        return ;
    }
    this->append_char_unlocked(c);
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void ft_string::append(const ft_string& string) noexcept
{
    ft_string::mutex_guard self_guard;
    ft_string::mutex_guard other_guard;
    int lock_error;
    size_t new_length;

    lock_error = ft_string::lock_pair(*this, string, self_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_system_error(lock_error);
        return ;
    }
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(this->_error_code);
        if (other_guard.owns_lock())
            other_guard.unlock();
        return ;
    }
    if (string._error_code != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(string._error_code);
        if (other_guard.owns_lock())
            other_guard.unlock();
        return ;
    }
    if (string._length == 0)
    {
        this->set_error_unlocked(FT_ERR_SUCCESSS);
        if (other_guard.owns_lock())
            other_guard.unlock();
        return ;
    }
    new_length = this->_length + string._length;
    if (new_length >= this->_capacity)
    {
        size_t new_capacity;

        new_capacity = this->_capacity;
        if (new_capacity == 0)
            new_capacity = 16;
        while (new_capacity <= new_length)
            new_capacity *= 2;
        this->resize_unlocked(new_capacity);
        if (this->_error_code != FT_ERR_SUCCESSS)
        {
            if (other_guard.owns_lock())
                other_guard.unlock();
            return ;
        }
    }
    ft_memcpy(this->_data + this->_length, string._data, string._length);
    this->_length = new_length;
    this->_data[this->_length] = '\0';
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    if (other_guard.owns_lock())
        other_guard.unlock();
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void ft_string::append(const char *string) noexcept
{
    size_t string_length;
    ft_string::mutex_guard guard;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_system_error(lock_error);
        return ;
    }
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(this->_error_code);
        return ;
    }
    if (!string)
    {
        this->set_error_unlocked(FT_ERR_SUCCESSS);
        return ;
    }
    string_length = ft_strlen_size_t(string);
    this->append_unlocked(string, string_length);
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void ft_string::clear() noexcept
{
    ft_string::mutex_guard guard;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_system_error(lock_error);
        return ;
    }
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(this->_error_code);
        return ;
    }
    this->clear_unlocked();
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

const char* ft_string::at(size_t index) const noexcept
{
    ft_string::mutex_guard guard;
    int lock_error;
    const char *result;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
        return (ft_nullptr);
    if (index >= this->_length)
    {
        return (ft_nullptr);
    }
    result = &this->_data[index];
    this->set_error(FT_ERR_SUCCESSS);
    return (result);
}

const char* ft_string::c_str() const noexcept
{
    ft_string::mutex_guard guard;
    int lock_error;
    const char *result;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
        return (const_cast<char *>(""));
    if (this->_data)
        result = this->_data;
    else
        result = const_cast<char *>("");
    this->set_error(FT_ERR_SUCCESSS);
    return (result);
}

char* ft_string::data() noexcept
{
    ft_string::mutex_guard guard;
    int lock_error;
    char *result;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_system_error(lock_error);
        return (ft_nullptr);
    }
    result = this->_data;
    this->set_error(FT_ERR_SUCCESSS);
    return (result);
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
    ft_string::mutex_guard guard;
    int lock_error;
    size_t length;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        return (0);
    }
    length = this->_length;
    this->set_error(FT_ERR_SUCCESSS);
    return (length);
}

bool ft_string::empty() const noexcept
{
    return (this->size() == 0);
}

int ft_string::get_error() const noexcept
{
    ft_string::mutex_guard guard;
    int lock_error;
    int error_code;
    int system_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
        return (lock_error);
    error_code = this->_error_code;
    system_error = this->_system_error_code;
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_errno = error_code;
        return (error_code);
    }
    if (system_error != FT_SYS_ERR_SUCCESS)
    {
        ft_errno = system_error;
        return (system_error);
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (FT_ERR_SUCCESSS);
}

const char* ft_string::get_error_str() const noexcept
{
    int lock_error;
    const char *error_string;

    {
        ft_string::mutex_guard guard;

        lock_error = this->lock_self(guard);
        if (lock_error != FT_ERR_SUCCESSS)
            return (ft_strerror(lock_error));
        if (this->_error_code != FT_ERR_SUCCESSS)
            error_string = ft_strerror(this->_error_code);
        else if (this->_system_error_code != FT_SYS_ERR_SUCCESS)
            error_string = ft_strerror(this->_system_error_code);
        else
            error_string = ft_strerror(FT_ERR_SUCCESSS);
    }
    if (this->_error_code != FT_ERR_SUCCESSS)
        ft_errno = this->_error_code;
    else
        ft_errno = this->_system_error_code;
    return (error_string);
}

void ft_string::reset_system_error() const noexcept
{
    ft_string::mutex_guard guard;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_system_error(lock_error);
        return ;
    }
    this->_system_error_code = FT_SYS_ERR_SUCCESS;
    ft_sys_errno = FT_SYS_ERR_SUCCESS;
    return ;
}

void ft_string::move(ft_string& other) noexcept
{
    ft_string::mutex_guard self_guard;
    ft_string::mutex_guard other_guard;
    int lock_error;

    lock_error = ft_string::lock_pair(*this, other, self_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_system_error(lock_error);
        return ;
    }
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(this->_error_code);
        if (other_guard.owns_lock())
            other_guard.unlock();
        return ;
    }
    this->move_unlocked(other);
    if (other_guard.owns_lock())
        other_guard.unlock();
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

ft_string& ft_string::operator+=(const ft_string& other) noexcept
{
    this->append(other);
    return (*this);
}

ft_string& ft_string::operator+=(const char* cstr) noexcept
{
    ft_string::mutex_guard guard;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_system_error(lock_error);
        return (*this);
    }
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(this->_error_code);
        return (*this);
    }
    if (cstr)
    {
        size_t index;

        index = 0;
        while (cstr[index] != '\0')
        {
            this->append_char_unlocked(cstr[index]);
            if (this->_error_code != FT_ERR_SUCCESSS)
                return (*this);
            index++;
        }
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (*this);
}

ft_string& ft_string::operator+=(char c) noexcept
{
    this->append(c);
    return (*this);
}

void ft_string::erase(std::size_t index, std::size_t count) noexcept
{
    ft_string::mutex_guard guard;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_system_error(lock_error);
        return ;
    }
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(this->_error_code);
        return ;
    }
    this->erase_unlocked(index, count);
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void ft_string::append(const char *string, size_t length) noexcept
{
    ft_string::mutex_guard guard;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_system_error(lock_error);
        return ;
    }
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(this->_error_code);
        return ;
    }
    this->append_unlocked(string, length);
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void ft_string::assign(size_t count, char character) noexcept
{
    ft_string::mutex_guard guard;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_system_error(lock_error);
        return ;
    }
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(this->_error_code);
        return ;
    }
    this->assign_unlocked(count, character);
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void ft_string::assign(const char *string, size_t length) noexcept
{
    ft_string::mutex_guard guard;
    int lock_error;

    this->set_error(FT_ERR_SUCCESSS);
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_system_error(lock_error);
        return ;
    }
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(this->_error_code);
        return ;
    }
    this->assign_unlocked(string, length);
    return ;
}

void ft_string::resize_length(size_t new_length) noexcept
{
    ft_string::mutex_guard guard;
    int lock_error;

    this->set_error(FT_ERR_SUCCESSS);
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_system_error(lock_error);
        return ;
    }
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(this->_error_code);
        return ;
    }
    this->resize_length_unlocked(new_length);
    return ;
}

void ft_string::push_back(char character) noexcept
{
    this->append(character);
    return ;
}

char ft_string::back() noexcept
{
    ft_string::mutex_guard guard;
    int lock_error;
    char value;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_system_error(lock_error);
        ft_errno = lock_error;
        return ('\0');
    }
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(this->_error_code);
        ft_errno = this->_error_code;
        return ('\0');
    }
    if (this->_length == 0)
    {
        this->set_error_unlocked(FT_ERR_OUT_OF_RANGE);
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return ('\0');
    }
    value = this->_data[this->_length - 1];
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    ft_errno = FT_ERR_SUCCESSS;
    return (value);
}

size_t ft_string::find(const char *substring) const noexcept
{
    ft_string::mutex_guard guard;
    int lock_error;
    size_t substring_length;
    size_t index;
    size_t match_index;
    size_t result;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_system_error(lock_error);
        ft_errno = lock_error;
        return (ft_string::npos);
    }
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(this->_error_code);
        ft_errno = this->_error_code;
        return (ft_string::npos);
    }
    if (!substring)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_string::npos);
    }
    substring_length = ft_strlen_size_t(substring);
    if (substring_length == 0)
    {
        this->set_error_unlocked(FT_ERR_SUCCESSS);
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    if (substring_length > this->_length)
    {
        this->set_error_unlocked(FT_ERR_SUCCESSS);
        ft_errno = FT_ERR_SUCCESSS;
        return (ft_string::npos);
    }
    index = 0;
    result = ft_string::npos;
    while (index + substring_length <= this->_length && result == ft_string::npos)
    {
        match_index = 0;
        while (match_index < substring_length
            && this->_data[index + match_index] == substring[match_index])
        {
            match_index++;
        }
        if (match_index == substring_length)
            result = index;
        else
            index++;
    }
    this->set_error(FT_ERR_SUCCESSS);
    ft_errno = FT_ERR_SUCCESSS;
    return (result);
}

size_t ft_string::find(const ft_string &substring) const noexcept
{
    ft_string::mutex_guard self_guard;
    ft_string::mutex_guard other_guard;
    int lock_error;
    size_t result;
    size_t index;
    size_t match_index;

    lock_error = ft_string::lock_pair(*this, substring, self_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_system_error(lock_error);
        ft_errno = lock_error;
        return (ft_string::npos);
    }
    if (this->_error_code != FT_ERR_SUCCESSS || substring._error_code != FT_ERR_SUCCESSS)
    {
        if (other_guard.owns_lock())
            other_guard.unlock();
        if (this->_error_code != FT_ERR_SUCCESSS)
        {
            this->set_error_unlocked(this->_error_code);
            ft_errno = this->_error_code;
        }
        else
        {
            this->set_error_unlocked(substring._error_code);
            ft_errno = substring._error_code;
        }
        return (ft_string::npos);
    }
    if (substring._length == 0)
    {
        if (other_guard.owns_lock())
            other_guard.unlock();
        this->set_error_unlocked(FT_ERR_SUCCESSS);
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    if (substring._length > this->_length)
    {
        if (other_guard.owns_lock())
            other_guard.unlock();
        this->set_error_unlocked(FT_ERR_SUCCESSS);
        ft_errno = FT_ERR_SUCCESSS;
        return (ft_string::npos);
    }
    result = ft_string::npos;
    index = 0;
    while (index + substring._length <= this->_length && result == ft_string::npos)
    {
        match_index = 0;
        while (match_index < substring._length
            && this->_data[index + match_index] == substring._data[match_index])
        {
            match_index++;
        }
        if (match_index == substring._length)
            result = index;
        else
            index++;
    }
    if (other_guard.owns_lock())
        other_guard.unlock();
    this->set_error(FT_ERR_SUCCESSS);
    ft_errno = FT_ERR_SUCCESSS;
    return (result);
}

ft_string ft_string::substr(size_t index, size_t count) const noexcept
{
    ft_string::mutex_guard guard;
    int lock_error;
    ft_string substring;
    size_t available_length;
    size_t copy_length;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        substring.set_system_error(lock_error);
        ft_errno = lock_error;
        return (substring);
    }
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        substring.set_error_unlocked(this->_error_code);
        ft_errno = this->_error_code;
        return (substring);
    }
    if (index > this->_length)
    {
        substring.set_error_unlocked(FT_ERR_OUT_OF_RANGE);
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (substring);
    }
    available_length = this->_length - index;
    copy_length = count;
    if (copy_length == ft_string::npos || copy_length > available_length)
        copy_length = available_length;
    if (copy_length > 0)
        substring.assign(this->_data + index, copy_length);
    this->set_error(FT_ERR_SUCCESSS);
    ft_errno = FT_ERR_SUCCESSS;
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

bool operator!=(const ft_string &lhs, const ft_string &rhs) noexcept
{
    return (ft_strcmp(lhs.c_str(), rhs.c_str()) != 0);
}

bool operator!=(const ft_string &lhs, const char* rhs) noexcept
{
    return (ft_strcmp(lhs.c_str(), rhs) != 0);
}

bool operator!=(const char* lhs, const ft_string &rhs) noexcept
{
    return (ft_strcmp(lhs, rhs.c_str()) != 0);
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

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex* ft_string::get_mutex_for_testing() noexcept
{
    ft_errno = FT_ERR_SUCCESSS;
    ft_sys_errno = FT_SYS_ERR_SUCCESS;
    return (&(this->_mutex));
}
#endif
