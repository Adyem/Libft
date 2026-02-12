#include "class_data_buffer.hpp"
#include "../Basic/basic.hpp"
#include <utility>
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/pthread_internal.hpp"

int DataBuffer::lock_mutex(void) const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_lock_with_error(*this->_mutex));
}

int DataBuffer::unlock_mutex(void) const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_unlock_with_error(*this->_mutex));
}

int DataBuffer::prepare_thread_safety(void) noexcept
{
    if (this->_mutex != ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    pt_recursive_mutex *mutex_pointer = ft_nullptr;
    int mutex_error = pt_recursive_mutex_create_with_error(&mutex_pointer);
    if (mutex_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(mutex_error);
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

void DataBuffer::teardown_thread_safety(void) noexcept
{
    pt_recursive_mutex_destroy(&this->_mutex);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

int DataBuffer::enable_thread_safety(void) noexcept
{
    return (this->prepare_thread_safety());
}

void DataBuffer::disable_thread_safety(void) noexcept
{
    this->teardown_thread_safety();
    return ;
}

bool DataBuffer::is_thread_safe_enabled(void) const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int DataBuffer::lock_pair(const DataBuffer &first, const DataBuffer &second,
        const DataBuffer *&lower, const DataBuffer *&upper) noexcept
{
    const DataBuffer *ordered_first;
    const DataBuffer *ordered_second;

    if (&first == &second)
    {
        lower = &first;
        upper = &first;
        return (first.lock_mutex());
    }
    ordered_first = &first;
    ordered_second = &second;
    if (ordered_first > ordered_second)
    {
        const DataBuffer *temporary = ordered_first;

        ordered_first = ordered_second;
        ordered_second = temporary;
    }
    lower = ordered_first;
    upper = ordered_second;
    while (true)
    {
        int lower_error = lower->lock_mutex();

        if (lower_error != FT_ERR_SUCCESS)
            return (lower_error);
        int upper_error = upper->lock_mutex();

        if (upper_error == FT_ERR_SUCCESS)
            return (FT_ERR_SUCCESS);
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            lower->unlock_mutex();
            return (upper_error);
        }
        lower->unlock_mutex();
        DataBuffer::sleep_backoff();
    }
}

int DataBuffer::unlock_pair(const DataBuffer *lower, const DataBuffer *upper) noexcept
{
    int error;
    int final_error = FT_ERR_SUCCESS;

    if (upper != ft_nullptr)
    {
        error = upper->unlock_mutex();
        if (error != FT_ERR_SUCCESS)
            final_error = error;
    }
    if (lower != ft_nullptr && lower != upper)
    {
        error = lower->unlock_mutex();
        if (error != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
            final_error = error;
    }
    return (final_error);
}

void DataBuffer::sleep_backoff() noexcept
{
    pt_thread_sleep(1);
    return ;
}

int DataBuffer::write_length_locked(size_t len) noexcept
{
    const uint8_t *ptr = reinterpret_cast<const uint8_t*>(&len);
    size_t index = 0;

    while (index < sizeof(size_t))
    {
        this->_buffer.push_back(ptr[index]);
        int buffer_error = ft_global_error_stack_peek_last_error();
        if (buffer_error != FT_ERR_SUCCESS)
            return (buffer_error);
        ++index;
    }
    return (FT_ERR_SUCCESS);
}

int DataBuffer::read_length_locked(size_t &len) noexcept
{
    if (this->_read_pos + sizeof(size_t) > this->_buffer.size())
        return (FT_ERR_INVALID_ARGUMENT);
    ft_memcpy(&len, this->_buffer.begin() + this->_read_pos, sizeof(size_t));
    this->_read_pos += sizeof(size_t);
    return (FT_ERR_SUCCESS);
}

DataBuffer::DataBuffer() noexcept
    : _buffer(), _read_pos(0), _ok(true), _mutex(ft_nullptr)
{
    this->enable_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

DataBuffer::DataBuffer(const DataBuffer &other) noexcept
    : _buffer(), _read_pos(0), _ok(true), _mutex(ft_nullptr)
{
    this->enable_thread_safety();
    int lock_error;
    const DataBuffer *lower = ft_nullptr;
    const DataBuffer *upper = ft_nullptr;
    int final_error = FT_ERR_SUCCESS;

    lock_error = DataBuffer::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    size_t other_size = other._buffer.size();
    size_t index = 0;
    while (index < other_size)
    {
        this->_buffer.push_back(other._buffer[index]);
        int buffer_error = ft_global_error_stack_peek_last_error();
        if (buffer_error != FT_ERR_SUCCESS)
        {
            this->_ok = false;
            final_error = buffer_error;
            break ;
        }
        ++index;
    }
    if (final_error == FT_ERR_SUCCESS)
    {
        this->_read_pos = other._read_pos;
        this->_ok = other._ok;
    }
    int unlock_error = DataBuffer::unlock_pair(lower, upper);
    if (unlock_error != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
        final_error = unlock_error;
    ft_global_error_stack_push(final_error);
    return ;
}

DataBuffer::DataBuffer(DataBuffer&& other) noexcept
    : _buffer(), _read_pos(0), _ok(true), _mutex(ft_nullptr)
{
    this->enable_thread_safety();
    int lock_error;
    const DataBuffer *lower = ft_nullptr;
    const DataBuffer *upper = ft_nullptr;
    int final_error = FT_ERR_SUCCESS;

    lock_error = DataBuffer::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->_buffer = ft_move(other._buffer);
    this->_read_pos = other._read_pos;
    this->_ok = other._ok;
    other._buffer.clear();
    other._read_pos = 0;
    other._ok = true;
    int unlock_error = DataBuffer::unlock_pair(lower, upper);
    if (unlock_error != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
        final_error = unlock_error;
    ft_global_error_stack_push(final_error);
    return ;
}

DataBuffer &DataBuffer::operator=(const DataBuffer &other) noexcept
{
    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (*this);
    }
    int lock_error;
    const DataBuffer *lower = ft_nullptr;
    const DataBuffer *upper = ft_nullptr;
    int final_error = FT_ERR_SUCCESS;

    lock_error = DataBuffer::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    this->_buffer.clear();
    size_t other_size = other._buffer.size();
    size_t index = 0;
    while (index < other_size)
    {
        this->_buffer.push_back(other._buffer[index]);
        int buffer_error = ft_global_error_stack_peek_last_error();
        if (buffer_error != FT_ERR_SUCCESS)
        {
            this->_ok = false;
            final_error = buffer_error;
            break ;
        }
        ++index;
    }
    if (final_error == FT_ERR_SUCCESS)
    {
        this->_read_pos = other._read_pos;
        this->_ok = other._ok;
    }
    int unlock_error = DataBuffer::unlock_pair(lower, upper);
    if (unlock_error != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
        final_error = unlock_error;
    ft_global_error_stack_push(final_error);
    return (*this);
}

DataBuffer &DataBuffer::operator=(DataBuffer &&other) noexcept
{
    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (*this);
    }
    int lock_error;
    const DataBuffer *lower = ft_nullptr;
    const DataBuffer *upper = ft_nullptr;
    int final_error = FT_ERR_SUCCESS;

    lock_error = DataBuffer::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    this->_buffer = ft_move(other._buffer);
    this->_read_pos = other._read_pos;
    this->_ok = other._ok;
    other._buffer.clear();
    other._read_pos = 0;
    other._ok = true;
    int unlock_error = DataBuffer::unlock_pair(lower, upper);
    if (unlock_error != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
        final_error = unlock_error;
    ft_global_error_stack_push(final_error);
    return (*this);
}

DataBuffer::~DataBuffer() noexcept
{
    this->disable_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

void DataBuffer::clear() noexcept
{
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->_buffer.clear();
    this->_read_pos = 0;
    this->_ok = true;
    int unlock_error = this->unlock_mutex();
    int final_error = FT_ERR_SUCCESS;
    if (unlock_error != FT_ERR_SUCCESS)
        final_error = unlock_error;
    ft_global_error_stack_push(final_error);
    return ;
}

size_t DataBuffer::size() const noexcept
{
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (0);
    }
    size_t result = this->_buffer.size();
    int unlock_error = this->unlock_mutex();
    int final_error = FT_ERR_SUCCESS;
    if (unlock_error != FT_ERR_SUCCESS)
        final_error = unlock_error;
    ft_global_error_stack_push(final_error);
    return (result);
}

const ft_vector<uint8_t>& DataBuffer::data() const noexcept
{
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (this->_buffer);
    }
    const ft_vector<uint8_t> *buffer_reference = &this->_buffer;
    int unlock_error = this->unlock_mutex();
    int final_error = FT_ERR_SUCCESS;
    if (unlock_error != FT_ERR_SUCCESS)
        final_error = unlock_error;
    ft_global_error_stack_push(final_error);
    return (*buffer_reference);
}

size_t DataBuffer::tell() const noexcept
{
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (0);
    }
    size_t position = this->_read_pos;
    int unlock_error = this->unlock_mutex();
    int final_error = FT_ERR_SUCCESS;
    if (unlock_error != FT_ERR_SUCCESS)
        final_error = unlock_error;
    ft_global_error_stack_push(final_error);
    return (position);
}

bool DataBuffer::seek(size_t pos) noexcept
{
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (false);
    }
    bool result;
    int final_error;

    if (pos <= this->_buffer.size())
    {
        this->_read_pos = pos;
        this->_ok = true;
        result = true;
        final_error = FT_ERR_SUCCESS;
    }
    else
    {
        this->_ok = false;
        result = false;
        final_error = FT_ERR_INVALID_ARGUMENT;
    }
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
        final_error = unlock_error;
    ft_global_error_stack_push(final_error);
    return (result);
}

DataBuffer::operator bool() const noexcept
{
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (false);
    }
    bool state = this->_ok;
    int final_error;

    if (state)
        final_error = FT_ERR_SUCCESS;
    else
        final_error = FT_ERR_INTERNAL;
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
        final_error = unlock_error;
    ft_global_error_stack_push(final_error);
    return (state);
}

bool DataBuffer::good() const noexcept
{
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (false);
    }
    bool state = this->_ok;
    int final_error;

    if (state)
        final_error = FT_ERR_SUCCESS;
    else
        final_error = FT_ERR_INTERNAL;
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
        final_error = unlock_error;
    ft_global_error_stack_push(final_error);
    return (state);
}

bool DataBuffer::bad() const noexcept
{
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (true);
    }
    bool result = !this->_ok;
    int final_error = FT_ERR_SUCCESS;
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
        final_error = unlock_error;
    ft_global_error_stack_push(final_error);
    return (result);
}

DataBuffer &DataBuffer::operator<<(size_t len)
{
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    int final_error = this->write_length_locked(len);
    if (final_error == FT_ERR_SUCCESS)
        this->_ok = true;
    else
        this->_ok = false;
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
        final_error = unlock_error;
    ft_global_error_stack_push(final_error);
    return (*this);
}

DataBuffer &DataBuffer::operator>>(size_t &len)
{
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    int final_error = this->read_length_locked(len);
    if (final_error == FT_ERR_SUCCESS)
        this->_ok = true;
    else
        this->_ok = false;
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
        final_error = unlock_error;
    ft_global_error_stack_push(final_error);
    return (*this);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *DataBuffer::get_mutex_for_validation() const noexcept
{
    return (&(this->_mutex));
}
#endif
