#include "class_data_buffer.hpp"
#include "../Libft/libft.hpp"
#include <utility>
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"

static int data_buffer_lock_mutex(const pt_recursive_mutex &mutex)
{
    int error = mutex.lock(THREAD_ID);
    ft_global_error_stack_pop_newest();
    return (error);
}

static int data_buffer_unlock_mutex(const pt_recursive_mutex &mutex)
{
    int error = mutex.unlock(THREAD_ID);
    ft_global_error_stack_pop_newest();
    return (error);
}

int DataBuffer::lock_self() const noexcept
{
    return (data_buffer_lock_mutex(this->_mutex));
}

int DataBuffer::unlock_self() const noexcept
{
    return (data_buffer_unlock_mutex(this->_mutex));
}

int DataBuffer::lock_pair(const DataBuffer &first, const DataBuffer &second,
        const DataBuffer *&lower, const DataBuffer *&upper) noexcept
{
    const DataBuffer *ordered_first = &first;
    const DataBuffer *ordered_second = &second;

    if (&first == &second)
    {
        lower = &first;
        upper = &first;
        return (data_buffer_lock_mutex(first._mutex));
    }
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
        int lower_error = data_buffer_lock_mutex(lower->_mutex);

        if (lower_error != FT_ERR_SUCCESSS)
            return (lower_error);
        int upper_error = data_buffer_lock_mutex(upper->_mutex);

        if (upper_error == FT_ERR_SUCCESSS)
            return (FT_ERR_SUCCESSS);
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            data_buffer_unlock_mutex(lower->_mutex);
            return (upper_error);
        }
        data_buffer_unlock_mutex(lower->_mutex);
        DataBuffer::sleep_backoff();
    }
}

int DataBuffer::unlock_pair(const DataBuffer *lower, const DataBuffer *upper) noexcept
{
    int error;
    int final_error = FT_ERR_SUCCESSS;

    if (upper != ft_nullptr)
    {
        error = data_buffer_unlock_mutex(upper->_mutex);
        if (error != FT_ERR_SUCCESSS)
            final_error = error;
    }
    if (lower != ft_nullptr && lower != upper)
    {
        error = data_buffer_unlock_mutex(lower->_mutex);
        if (error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
            final_error = error;
    }
    return (final_error);
}

void DataBuffer::sleep_backoff() noexcept
{
    pt_thread_sleep(1);
    return ;
}

void DataBuffer::record_operation_error(int error_code) const
{
    unsigned long long operation_id = ft_errno_next_operation_id();

    ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    ft_operation_error_stack_push(&this->_operation_errors, error_code, operation_id);
}

int DataBuffer::write_length_locked(size_t len) noexcept
{
    const uint8_t *ptr = reinterpret_cast<const uint8_t*>(&len);
    size_t index = 0;

    while (index < sizeof(size_t))
    {
        this->_buffer.push_back(ptr[index]);
        int buffer_error = ft_global_error_stack_last_error();
        if (buffer_error != FT_ERR_SUCCESSS)
            return (buffer_error);
        ++index;
    }
    return (FT_ERR_SUCCESSS);
}

int DataBuffer::read_length_locked(size_t &len) noexcept
{
    if (this->_read_pos + sizeof(size_t) > this->_buffer.size())
        return (FT_ERR_INVALID_ARGUMENT);
    ft_memcpy(&len, this->_buffer.begin() + this->_read_pos, sizeof(size_t));
    this->_read_pos += sizeof(size_t);
    return (FT_ERR_SUCCESSS);
}

DataBuffer::DataBuffer() noexcept
    : _buffer(), _read_pos(0), _ok(true), _mutex()
{
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

DataBuffer::DataBuffer(const DataBuffer &other) noexcept
    : _buffer(), _read_pos(0), _ok(true), _mutex()
{
    int lock_error;
    const DataBuffer *lower = ft_nullptr;
    const DataBuffer *upper = ft_nullptr;
    int final_error = FT_ERR_SUCCESSS;

    lock_error = DataBuffer::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return ;
    }
    size_t other_size = other._buffer.size();
    size_t index = 0;
    while (index < other_size)
    {
        this->_buffer.push_back(other._buffer[index]);
        int buffer_error = ft_global_error_stack_last_error();
        if (buffer_error != FT_ERR_SUCCESSS)
        {
            this->_ok = false;
            final_error = buffer_error;
            break ;
        }
        ++index;
    }
    if (final_error == FT_ERR_SUCCESSS)
    {
        this->_read_pos = other._read_pos;
        this->_ok = other._ok;
    }
    int unlock_error = DataBuffer::unlock_pair(lower, upper);
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    this->record_operation_error(final_error);
    return ;
}

DataBuffer::DataBuffer(DataBuffer&& other) noexcept
    : _buffer(), _read_pos(0), _ok(true), _mutex()
{
    int lock_error;
    const DataBuffer *lower = ft_nullptr;
    const DataBuffer *upper = ft_nullptr;
    int final_error = FT_ERR_SUCCESSS;

    lock_error = DataBuffer::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return ;
    }
    this->_buffer = ft_move(other._buffer);
    this->_read_pos = other._read_pos;
    this->_ok = other._ok;
    other._buffer.clear();
    other._read_pos = 0;
    other._ok = true;
    int unlock_error = DataBuffer::unlock_pair(lower, upper);
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    this->record_operation_error(final_error);
    return ;
}

DataBuffer &DataBuffer::operator=(const DataBuffer &other) noexcept
{
    if (this == &other)
    {
        this->record_operation_error(FT_ERR_SUCCESSS);
        return (*this);
    }
    int lock_error;
    const DataBuffer *lower = ft_nullptr;
    const DataBuffer *upper = ft_nullptr;
    int final_error = FT_ERR_SUCCESSS;

    lock_error = DataBuffer::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (*this);
    }
    this->_buffer.clear();
    size_t other_size = other._buffer.size();
    size_t index = 0;
    while (index < other_size)
    {
        this->_buffer.push_back(other._buffer[index]);
        int buffer_error = ft_global_error_stack_last_error();
        if (buffer_error != FT_ERR_SUCCESSS)
        {
            this->_ok = false;
            final_error = buffer_error;
            break ;
        }
        ++index;
    }
    if (final_error == FT_ERR_SUCCESSS)
    {
        this->_read_pos = other._read_pos;
        this->_ok = other._ok;
    }
    int unlock_error = DataBuffer::unlock_pair(lower, upper);
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    this->record_operation_error(final_error);
    return (*this);
}

DataBuffer &DataBuffer::operator=(DataBuffer &&other) noexcept
{
    if (this == &other)
    {
        this->record_operation_error(FT_ERR_SUCCESSS);
        return (*this);
    }
    int lock_error;
    const DataBuffer *lower = ft_nullptr;
    const DataBuffer *upper = ft_nullptr;
    int final_error = FT_ERR_SUCCESSS;

    lock_error = DataBuffer::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (*this);
    }
    this->_buffer = ft_move(other._buffer);
    this->_read_pos = other._read_pos;
    this->_ok = other._ok;
    other._buffer.clear();
    other._read_pos = 0;
    other._ok = true;
    int unlock_error = DataBuffer::unlock_pair(lower, upper);
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    this->record_operation_error(final_error);
    return (*this);
}

DataBuffer::~DataBuffer() noexcept
{
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

void DataBuffer::clear() noexcept
{
    int lock_error = this->lock_self();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return ;
    }
    this->_buffer.clear();
    this->_read_pos = 0;
    this->_ok = true;
    int unlock_error = data_buffer_unlock_mutex(this->_mutex);
    int final_error = FT_ERR_SUCCESSS;
    if (unlock_error != FT_ERR_SUCCESSS)
        final_error = unlock_error;
    this->record_operation_error(final_error);
    return ;
}

size_t DataBuffer::size() const noexcept
{
    int lock_error = this->lock_self();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0);
    }
    size_t result = this->_buffer.size();
    int unlock_error = data_buffer_unlock_mutex(this->_mutex);
    int final_error = FT_ERR_SUCCESSS;
    if (unlock_error != FT_ERR_SUCCESSS)
        final_error = unlock_error;
    this->record_operation_error(final_error);
    return (result);
}

const ft_vector<uint8_t>& DataBuffer::data() const noexcept
{
    int lock_error = this->lock_self();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (this->_buffer);
    }
    const ft_vector<uint8_t> *buffer_reference = &this->_buffer;
    int unlock_error = data_buffer_unlock_mutex(this->_mutex);
    int final_error = FT_ERR_SUCCESSS;
    if (unlock_error != FT_ERR_SUCCESSS)
        final_error = unlock_error;
    this->record_operation_error(final_error);
    return (*buffer_reference);
}

size_t DataBuffer::tell() const noexcept
{
    int lock_error = this->lock_self();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0);
    }
    size_t position = this->_read_pos;
    int unlock_error = data_buffer_unlock_mutex(this->_mutex);
    int final_error = FT_ERR_SUCCESSS;
    if (unlock_error != FT_ERR_SUCCESSS)
        final_error = unlock_error;
    this->record_operation_error(final_error);
    return (position);
}

bool DataBuffer::seek(size_t pos) noexcept
{
    int lock_error = this->lock_self();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (false);
    }
    bool result;
    int final_error;

    if (pos <= this->_buffer.size())
    {
        this->_read_pos = pos;
        this->_ok = true;
        result = true;
        final_error = FT_ERR_SUCCESSS;
    }
    else
    {
        this->_ok = false;
        result = false;
        final_error = FT_ERR_INVALID_ARGUMENT;
    }
    int unlock_error = data_buffer_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    this->record_operation_error(final_error);
    return (result);
}

DataBuffer::operator bool() const noexcept
{
    int lock_error = this->lock_self();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (false);
    }
    bool state = this->_ok;
    int final_error = state ? FT_ERR_SUCCESSS : FT_ERR_INTERNAL;
    int unlock_error = data_buffer_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    this->record_operation_error(final_error);
    return (state);
}

bool DataBuffer::good() const noexcept
{
    int lock_error = this->lock_self();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (false);
    }
    bool state = this->_ok;
    int final_error = state ? FT_ERR_SUCCESSS : FT_ERR_INTERNAL;
    int unlock_error = data_buffer_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    this->record_operation_error(final_error);
    return (state);
}

bool DataBuffer::bad() const noexcept
{
    int lock_error = this->lock_self();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (true);
    }
    bool result = !this->_ok;
    int final_error = FT_ERR_SUCCESSS;
    int unlock_error = data_buffer_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    this->record_operation_error(final_error);
    return (result);
}

DataBuffer &DataBuffer::operator<<(size_t len)
{
    int lock_error = this->lock_self();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (*this);
    }
    int final_error = this->write_length_locked(len);
    if (final_error == FT_ERR_SUCCESSS)
        this->_ok = true;
    else
        this->_ok = false;
    int unlock_error = data_buffer_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    this->record_operation_error(final_error);
    return (*this);
}

DataBuffer &DataBuffer::operator>>(size_t &len)
{
    int lock_error = this->lock_self();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (*this);
    }
    int final_error = this->read_length_locked(len);
    if (final_error == FT_ERR_SUCCESSS)
        this->_ok = true;
    else
        this->_ok = false;
    int unlock_error = data_buffer_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    this->record_operation_error(final_error);
    return (*this);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *DataBuffer::get_mutex_for_validation() const noexcept
{
    return (&(this->_mutex));
}

ft_operation_error_stack *DataBuffer::operation_error_stack_handle() const noexcept
{
    return (&(this->_operation_errors));
}
#endif
