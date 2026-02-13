#include "class_data_buffer.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

thread_local int32_t DataBuffer::_last_error = FT_ERR_SUCCESS;

data_buffer_proxy::data_buffer_proxy() noexcept
    : _data_buffer(ft_nullptr), _error_code(FT_ERR_SUCCESS)
{
    return ;
}

data_buffer_proxy::data_buffer_proxy(DataBuffer *data_buffer, int32_t error_code) noexcept
    : _data_buffer(data_buffer), _error_code(error_code)
{
    return ;
}

data_buffer_proxy::data_buffer_proxy(const data_buffer_proxy &other) noexcept
    : _data_buffer(other._data_buffer), _error_code(other._error_code)
{
    return ;
}

data_buffer_proxy::data_buffer_proxy(data_buffer_proxy &&other) noexcept
    : _data_buffer(other._data_buffer), _error_code(other._error_code)
{
    other._data_buffer = ft_nullptr;
    other._error_code = FT_ERR_SUCCESS;
    return ;
}

data_buffer_proxy::~data_buffer_proxy()
{
    return ;
}

data_buffer_proxy &data_buffer_proxy::operator=(const data_buffer_proxy &other) noexcept
{
    this->_data_buffer = other._data_buffer;
    this->_error_code = other._error_code;
    return (*this);
}

data_buffer_proxy &data_buffer_proxy::operator=(data_buffer_proxy &&other) noexcept
{
    this->_data_buffer = other._data_buffer;
    this->_error_code = other._error_code;
    other._data_buffer = ft_nullptr;
    other._error_code = FT_ERR_SUCCESS;
    return (*this);
}

int32_t data_buffer_proxy::get_error() const noexcept
{
    return (this->_error_code);
}

void DataBuffer::abort_lifecycle_error(const char *method_name, const char *reason) noexcept
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "DataBuffer lifecycle error: %s: %s\n", method_name, reason);
    su_abort();
    return ;
}

void DataBuffer::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == DataBuffer::_state_initialized)
        return ;
    DataBuffer::abort_lifecycle_error(method_name, "called while object is not initialized");
    return ;
}

int32_t DataBuffer::set_last_operation_error(int32_t error_code) noexcept
{
    DataBuffer::_last_error = error_code;
    return (error_code);
}

void DataBuffer::set_operation_error(int32_t error_code) noexcept
{
    this->_operation_error = error_code;
    this->_ok = (error_code == FT_ERR_SUCCESS);
    DataBuffer::set_last_operation_error(error_code);
    return ;
}

int DataBuffer::lock_mutex(void) const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->lock());
}

int DataBuffer::unlock_mutex(void) const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

int DataBuffer::write_length_locked(size_t length) noexcept
{
    const uint8_t *pointer = reinterpret_cast<const uint8_t *>(&length);
    size_t index = 0;

    while (index < sizeof(size_t))
    {
        this->_buffer.push_back(pointer[index]);
        int push_error = ft_vector<uint8_t>::last_operation_error();
        if (push_error != FT_ERR_SUCCESS)
            return (push_error);
        index++;
    }
    return (FT_ERR_SUCCESS);
}

int DataBuffer::read_length_locked(size_t &length) noexcept
{
    if (this->_read_pos + sizeof(size_t) > this->_buffer.size())
        return (FT_ERR_INVALID_ARGUMENT);
    ft_memcpy(&length, this->_buffer.begin() + this->_read_pos, sizeof(size_t));
    this->_read_pos += sizeof(size_t);
    return (FT_ERR_SUCCESS);
}

DataBuffer::DataBuffer() noexcept
    : _buffer(), _read_pos(0), _ok(true), _mutex(ft_nullptr),
      _initialized_state(DataBuffer::_state_uninitialized), _operation_error(FT_ERR_SUCCESS)
{
    return ;
}

DataBuffer::~DataBuffer() noexcept
{
    if (this->_initialized_state == DataBuffer::_state_uninitialized)
    {
        DataBuffer::abort_lifecycle_error("DataBuffer::~DataBuffer",
            "destructor called while object is uninitialized");
        return ;
    }
    if (this->_initialized_state == DataBuffer::_state_initialized)
    {
        int destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            DataBuffer::set_last_operation_error(destroy_error);
    }
    return ;
}

int DataBuffer::initialize() noexcept
{
    if (this->_initialized_state == DataBuffer::_state_initialized)
    {
        DataBuffer::abort_lifecycle_error("DataBuffer::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_buffer.clear();
    this->_read_pos = 0;
    this->_ok = true;
    this->_operation_error = FT_ERR_SUCCESS;
    this->_initialized_state = DataBuffer::_state_initialized;
    DataBuffer::set_last_operation_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int DataBuffer::initialize(const DataBuffer &other) noexcept
{
    if (other._initialized_state != DataBuffer::_state_initialized)
    {
        DataBuffer::abort_lifecycle_error("DataBuffer::initialize(const DataBuffer &)",
            "source is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (&other == this)
    {
        this->set_operation_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    if (this->_initialized_state == DataBuffer::_state_initialized)
    {
        int destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (destroy_error);
    }
    this->_read_pos = 0;
    this->_ok = true;
    this->_operation_error = FT_ERR_SUCCESS;
    this->_initialized_state = DataBuffer::_state_initialized;
    int self_lock_error = this->lock_mutex();
    if (self_lock_error != FT_ERR_SUCCESS)
        return (self_lock_error);
    int other_lock_error = other.lock_mutex();
    if (other_lock_error != FT_ERR_SUCCESS)
    {
        this->unlock_mutex();
        return (other_lock_error);
    }
    this->_buffer.clear();
    int copy_error = FT_ERR_SUCCESS;
    size_t index = 0;
    while (index < other._buffer.size() && copy_error == FT_ERR_SUCCESS)
    {
        this->_buffer.push_back(other._buffer[index]);
        copy_error = ft_vector<uint8_t>::last_operation_error();
        index++;
    }
    if (copy_error == FT_ERR_SUCCESS)
        this->_read_pos = other._read_pos;
    if (copy_error == FT_ERR_SUCCESS)
        this->_ok = other._ok;
    int other_unlock_error = other.unlock_mutex();
    int self_unlock_error = this->unlock_mutex();
    int final_error = copy_error;
    if (other_unlock_error != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
        final_error = other_unlock_error;
    if (self_unlock_error != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
        final_error = self_unlock_error;
    this->set_operation_error(final_error);
    return (final_error);
}

int DataBuffer::initialize_move(DataBuffer &other) noexcept
{
    if (&other == this)
    {
        if (this->_initialized_state != DataBuffer::_state_initialized)
        {
            DataBuffer::abort_lifecycle_error("DataBuffer::initialize_move(DataBuffer &)",
                "self move on uninitialized object");
            return (FT_ERR_INVALID_STATE);
        }
        this->set_operation_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    if (other._initialized_state != DataBuffer::_state_initialized)
    {
        DataBuffer::abort_lifecycle_error("DataBuffer::initialize_move(DataBuffer &)",
            "source is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this->_initialized_state == DataBuffer::_state_initialized)
    {
        int destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (destroy_error);
    }
    this->_read_pos = 0;
    this->_ok = true;
    this->_operation_error = FT_ERR_SUCCESS;
    this->_initialized_state = DataBuffer::_state_initialized;
    int self_lock_error = this->lock_mutex();
    if (self_lock_error != FT_ERR_SUCCESS)
        return (self_lock_error);
    int other_lock_error = other.lock_mutex();
    if (other_lock_error != FT_ERR_SUCCESS)
    {
        this->unlock_mutex();
        return (other_lock_error);
    }
    this->_buffer.clear();
    int move_error = FT_ERR_SUCCESS;
    size_t index = 0;
    while (index < other._buffer.size() && move_error == FT_ERR_SUCCESS)
    {
        this->_buffer.push_back(other._buffer[index]);
        move_error = ft_vector<uint8_t>::last_operation_error();
        index++;
    }
    if (move_error == FT_ERR_SUCCESS)
        this->_read_pos = other._read_pos;
    if (move_error == FT_ERR_SUCCESS)
        this->_ok = other._ok;
    if (move_error == FT_ERR_SUCCESS)
        other._buffer.clear();
    if (move_error == FT_ERR_SUCCESS)
        other._read_pos = 0;
    if (move_error == FT_ERR_SUCCESS)
        other._ok = true;
    int other_unlock_error = other.unlock_mutex();
    int self_unlock_error = this->unlock_mutex();
    int final_error = move_error;
    if (other_unlock_error != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
        final_error = other_unlock_error;
    if (self_unlock_error != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
        final_error = self_unlock_error;
    this->set_operation_error(final_error);
    return (final_error);
}

int DataBuffer::destroy() noexcept
{
    if (this->_initialized_state != DataBuffer::_state_initialized)
    {
        DataBuffer::abort_lifecycle_error("DataBuffer::destroy",
            "called while object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_buffer.clear();
    this->_read_pos = 0;
    this->_ok = true;
    if (this->_mutex != ft_nullptr)
    {
        int destroy_error = this->_mutex->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            this->_operation_error = destroy_error;
        delete this->_mutex;
        this->_mutex = ft_nullptr;
    }
    this->_operation_error = FT_ERR_SUCCESS;
    this->_initialized_state = DataBuffer::_state_destroyed;
    DataBuffer::set_last_operation_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

void DataBuffer::clear() noexcept
{
    this->abort_if_not_initialized("DataBuffer::clear");
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_operation_error(lock_error);
        return ;
    }
    this->_buffer.clear();
    this->_read_pos = 0;
    int unlock_error = this->unlock_mutex();
    this->set_operation_error(unlock_error);
    return ;
}

size_t DataBuffer::size() const noexcept
{
    this->abort_if_not_initialized("DataBuffer::size");
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<DataBuffer *>(this)->set_operation_error(lock_error);
        return (0);
    }
    size_t value = this->_buffer.size();
    int unlock_error = this->unlock_mutex();
    const_cast<DataBuffer *>(this)->set_operation_error(unlock_error);
    return (value);
}

const ft_vector<uint8_t>& DataBuffer::data() const noexcept
{
    this->abort_if_not_initialized("DataBuffer::data");
    const_cast<DataBuffer *>(this)->set_operation_error(FT_ERR_SUCCESS);
    return (this->_buffer);
}

size_t DataBuffer::tell() const noexcept
{
    this->abort_if_not_initialized("DataBuffer::tell");
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<DataBuffer *>(this)->set_operation_error(lock_error);
        return (0);
    }
    size_t value = this->_read_pos;
    int unlock_error = this->unlock_mutex();
    const_cast<DataBuffer *>(this)->set_operation_error(unlock_error);
    return (value);
}

bool DataBuffer::seek(size_t position) noexcept
{
    this->abort_if_not_initialized("DataBuffer::seek");
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_operation_error(lock_error);
        return (false);
    }
    int operation_error = FT_ERR_SUCCESS;
    if (position > this->_buffer.size())
        operation_error = FT_ERR_INVALID_ARGUMENT;
    if (operation_error == FT_ERR_SUCCESS)
        this->_read_pos = position;
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS && operation_error == FT_ERR_SUCCESS)
        operation_error = unlock_error;
    this->set_operation_error(operation_error);
    return (operation_error == FT_ERR_SUCCESS);
}

DataBuffer::operator bool() const noexcept
{
    this->abort_if_not_initialized("DataBuffer::operator bool");
    const_cast<DataBuffer *>(this)->set_operation_error(FT_ERR_SUCCESS);
    return (this->_ok);
}

bool DataBuffer::good() const noexcept
{
    this->abort_if_not_initialized("DataBuffer::good");
    const_cast<DataBuffer *>(this)->set_operation_error(FT_ERR_SUCCESS);
    return (this->_ok);
}

bool DataBuffer::bad() const noexcept
{
    this->abort_if_not_initialized("DataBuffer::bad");
    const_cast<DataBuffer *>(this)->set_operation_error(FT_ERR_SUCCESS);
    return (!this->_ok);
}

int DataBuffer::enable_thread_safety(void) noexcept
{
    this->abort_if_not_initialized("DataBuffer::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
    {
        this->set_operation_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    pt_recursive_mutex *mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        this->set_operation_error(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    int initialization_error = mutex_pointer->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        this->set_operation_error(initialization_error);
        return (initialization_error);
    }
    this->_mutex = mutex_pointer;
    this->set_operation_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

void DataBuffer::disable_thread_safety(void) noexcept
{
    this->abort_if_not_initialized("DataBuffer::disable_thread_safety");
    if (this->_mutex != ft_nullptr)
    {
        int destroy_error = this->_mutex->destroy();
        (void)destroy_error;
        delete this->_mutex;
        this->_mutex = ft_nullptr;
    }
    this->set_operation_error(FT_ERR_SUCCESS);
    return ;
}

bool DataBuffer::is_thread_safe(void) const noexcept
{
    this->abort_if_not_initialized("DataBuffer::is_thread_safe");
    const_cast<DataBuffer *>(this)->set_operation_error(FT_ERR_SUCCESS);
    return (this->_mutex != ft_nullptr);
}

int32_t DataBuffer::last_operation_error() noexcept
{
    return (DataBuffer::_last_error);
}

const char *DataBuffer::last_operation_error_str() noexcept
{
    return (ft_strerror(DataBuffer::last_operation_error()));
}

int32_t DataBuffer::get_operation_error() const noexcept
{
    return (this->_operation_error);
}

const char *DataBuffer::get_operation_error_str() const noexcept
{
    return (ft_strerror(this->_operation_error));
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *DataBuffer::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif
