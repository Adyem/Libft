#include "class_data_buffer.hpp"
#include "../Libft/libft.hpp"
#include <utility>
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"

void DataBuffer::sleep_backoff() noexcept
{
    pt_thread_sleep(1);
    return ;
}

void DataBuffer::restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno) noexcept
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    return ;
}

void DataBuffer::set_error_unlocked(int error_code) const noexcept
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

void DataBuffer::set_error(int error_code) const noexcept
{
    this->set_error_unlocked(error_code);
    return ;
}

int DataBuffer::lock_self(ft_unique_lock<pt_mutex> &guard) const noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> local_guard(this->_mutex);
    if (local_guard.get_error() != ER_SUCCESS)
    {
        ft_errno = entry_errno;
        guard = ft_unique_lock<pt_mutex>();
        return (local_guard.get_error());
    }
    ft_errno = entry_errno;
    guard = ft_move(local_guard);
    return (ER_SUCCESS);
}

int DataBuffer::lock_pair(const DataBuffer &first, const DataBuffer &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard) noexcept
{
    const DataBuffer *ordered_first;
    const DataBuffer *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = ER_SUCCESS;
        return (ER_SUCCESS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const DataBuffer *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == ER_SUCCESS)
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
            ft_errno = ER_SUCCESS;
            return (ER_SUCCESS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        DataBuffer::sleep_backoff();
    }
}

int DataBuffer::write_length_locked(size_t len) noexcept
{
    const uint8_t *ptr;
    size_t index;

    ptr = reinterpret_cast<const uint8_t*>(&len);
    index = 0;
    while (index < sizeof(size_t))
    {
        this->_buffer.push_back(ptr[index]);
        if (this->_buffer.get_error() != ER_SUCCESS)
            return (this->_buffer.get_error());
        ++index;
    }
    return (ER_SUCCESS);
}

int DataBuffer::read_length_locked(size_t &len) noexcept
{
    if (this->_read_pos + sizeof(size_t) > this->_buffer.size())
        return (FT_ERR_INVALID_ARGUMENT);
    ft_memcpy(&len, this->_buffer.begin() + this->_read_pos, sizeof(size_t));
    this->_read_pos += sizeof(size_t);
    return (ER_SUCCESS);
}

DataBuffer::DataBuffer() noexcept
    : _buffer(), _read_pos(0), _ok(true), _error_code(ER_SUCCESS), _mutex()
{
    return ;
}

DataBuffer::DataBuffer(const DataBuffer& other) noexcept
    : _buffer(), _read_pos(0), _ok(true), _error_code(ER_SUCCESS), _mutex()
{
    *this = other;
    return ;
}

DataBuffer::DataBuffer(DataBuffer&& other) noexcept
    : _buffer(), _read_pos(0), _ok(true), _error_code(ER_SUCCESS), _mutex()
{
    *this = ft_move(other);
    return ;
}

DataBuffer& DataBuffer::operator=(const DataBuffer& other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;
    size_t index;
    size_t other_size;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = DataBuffer::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        return (*this);
    }
    this->_buffer.clear();
    other_size = other._buffer.size();
    index = 0;
    while (index < other_size)
    {
        this->_buffer.push_back(other._buffer[index]);
        if (this->_buffer.get_error() != ER_SUCCESS)
        {
            this->_ok = false;
            this->set_error_unlocked(this->_buffer.get_error());
            DataBuffer::restore_errno(this_guard, entry_errno);
            DataBuffer::restore_errno(other_guard, entry_errno);
            return (*this);
        }
        ++index;
    }
    this->_read_pos = other._read_pos;
    this->_ok = other._ok;
    this->_error_code = other._error_code;
    this->set_error_unlocked(other._error_code);
    DataBuffer::restore_errno(this_guard, entry_errno);
    DataBuffer::restore_errno(other_guard, entry_errno);
    return (*this);
}

DataBuffer& DataBuffer::operator=(DataBuffer&& other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = DataBuffer::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        return (*this);
    }
    this->_buffer = ft_move(other._buffer);
    this->_read_pos = other._read_pos;
    this->_ok = other._ok;
    this->_error_code = other._error_code;
    this->set_error_unlocked(other._error_code);
    other._read_pos = 0;
    other._ok = true;
    other._error_code = ER_SUCCESS;
    other.set_error_unlocked(ER_SUCCESS);
    DataBuffer::restore_errno(this_guard, entry_errno);
    DataBuffer::restore_errno(other_guard, entry_errno);
    return (*this);
}

DataBuffer::~DataBuffer() noexcept
{
    return ;
}

void DataBuffer::clear() noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return ;
    }
    this->_buffer.clear();
    this->_read_pos = 0;
    this->_ok = true;
    this->set_error_unlocked(ER_SUCCESS);
    DataBuffer::restore_errno(guard, entry_errno);
    return ;
}

size_t DataBuffer::size() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    size_t result;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (0);
    }
    result = this->_buffer.size();
    this->set_error_unlocked(ER_SUCCESS);
    DataBuffer::restore_errno(guard, entry_errno);
    return (result);
}

const ft_vector<uint8_t>& DataBuffer::data() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    const ft_vector<uint8_t> *buffer_reference;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (this->_buffer);
    }
    buffer_reference = &this->_buffer;
    this->set_error_unlocked(ER_SUCCESS);
    DataBuffer::restore_errno(guard, entry_errno);
    return (*buffer_reference);
}

size_t DataBuffer::tell() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    size_t position;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (0);
    }
    position = this->_read_pos;
    this->set_error_unlocked(ER_SUCCESS);
    DataBuffer::restore_errno(guard, entry_errno);
    return (position);
}

bool DataBuffer::seek(size_t pos) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (false);
    }
    if (pos <= this->_buffer.size())
    {
        this->_read_pos = pos;
        this->_ok = true;
        this->set_error_unlocked(ER_SUCCESS);
        DataBuffer::restore_errno(guard, entry_errno);
        return (true);
    }
    this->_ok = false;
    this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
    DataBuffer::restore_errno(guard, entry_errno);
    return (false);
}

DataBuffer::operator bool() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    bool state;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (false);
    }
    state = this->_ok;
    if (state)
        this->set_error_unlocked(ER_SUCCESS);
    DataBuffer::restore_errno(guard, entry_errno);
    return (state);
}

bool DataBuffer::good() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    bool state;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (false);
    }
    state = this->_ok;
    if (state)
        this->set_error_unlocked(ER_SUCCESS);
    DataBuffer::restore_errno(guard, entry_errno);
    return (state);
}

bool DataBuffer::bad() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    bool is_bad;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (true);
    }
    if (this->_ok)
    {
        this->set_error_unlocked(ER_SUCCESS);
        DataBuffer::restore_errno(guard, entry_errno);
        return (false);
    }
    is_bad = true;
    DataBuffer::restore_errno(guard, entry_errno);
    return (is_bad);
}

DataBuffer& DataBuffer::operator<<(size_t len)
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    int write_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (*this);
    }
    write_error = this->write_length_locked(len);
    if (write_error != ER_SUCCESS)
    {
        this->_ok = false;
        this->set_error_unlocked(write_error);
        DataBuffer::restore_errno(guard, entry_errno);
        return (*this);
    }
    this->_ok = true;
    this->set_error_unlocked(ER_SUCCESS);
    DataBuffer::restore_errno(guard, entry_errno);
    return (*this);
}

DataBuffer& DataBuffer::operator>>(size_t& len)
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    int read_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (*this);
    }
    read_error = this->read_length_locked(len);
    if (read_error != ER_SUCCESS)
    {
        this->_ok = false;
        this->set_error_unlocked(read_error);
        DataBuffer::restore_errno(guard, entry_errno);
        return (*this);
    }
    this->_ok = true;
    this->set_error_unlocked(ER_SUCCESS);
    DataBuffer::restore_errno(guard, entry_errno);
    return (*this);
}

int DataBuffer::get_error() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    int error_value;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (lock_error);
    }
    error_value = this->_error_code;
    DataBuffer::restore_errno(guard, entry_errno);
    return (error_value);
}

const char *DataBuffer::get_error_str() const noexcept
{
    int error_value;

    error_value = this->get_error();
    return (ft_strerror(error_value));
}

