#ifndef DATA_BUFFER
# define DATA_BUFFER

#include "../Template/vector.hpp"
#include "../CMA/CMA.hpp"
#include "../Basic/basic.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Advanced/advanced.hpp"
#include "class_nullptr.hpp"
#include "class_string.hpp"
#include "class_istringstream.hpp"
#include <cstdint>
#include <sstream>

class DataBuffer;

class data_buffer_proxy
{
    private:
        DataBuffer *_data_buffer;
        int32_t _error_code;

    public:
        data_buffer_proxy() noexcept;
        data_buffer_proxy(DataBuffer *data_buffer, int32_t error_code) noexcept;
        data_buffer_proxy(const data_buffer_proxy &other) noexcept;
        data_buffer_proxy(data_buffer_proxy &&other) noexcept;
        ~data_buffer_proxy();

        data_buffer_proxy &operator=(const data_buffer_proxy &other) noexcept;
        data_buffer_proxy &operator=(data_buffer_proxy &&other) noexcept;

        template <typename ValueType>
        data_buffer_proxy operator<<(const ValueType &value) const noexcept;

        template <typename ValueType>
        data_buffer_proxy operator>>(ValueType &value) const noexcept;

        int32_t get_error() const noexcept;
};

class DataBuffer
{
    friend class data_buffer_proxy;

    private:
        ft_vector<uint8_t> _buffer;
        size_t _read_pos;
        bool _ok;
        mutable pt_recursive_mutex *_mutex;
        uint8_t _initialized_state;
        int32_t _operation_error;
        static thread_local int32_t _last_error;
        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        static void abort_lifecycle_error(const char *method_name, const char *reason) noexcept;
        void abort_if_not_initialized(const char *method_name) const noexcept;
        static int32_t set_last_operation_error(int32_t error_code) noexcept;
        void set_operation_error(int32_t error_code) noexcept;
        int lock_mutex(void) const noexcept;
        int unlock_mutex(void) const noexcept;
        int write_length_locked(size_t length) noexcept;
        int read_length_locked(size_t &length) noexcept;

        template <typename ValueType>
        int write_value(const ValueType &value) noexcept;

        template <typename ValueType>
        int read_value(ValueType &value) noexcept;

    public:
        DataBuffer() noexcept;
        ~DataBuffer() noexcept;

        DataBuffer(const DataBuffer& other) noexcept = delete;
        DataBuffer(DataBuffer&& other) noexcept = delete;
        DataBuffer& operator=(const DataBuffer& other) noexcept = delete;
        DataBuffer& operator=(DataBuffer&& other) noexcept = delete;

        int initialize() noexcept;
        int initialize(const DataBuffer &other) noexcept;
        int initialize_move(DataBuffer &other) noexcept;
        int destroy() noexcept;

        void clear() noexcept;
        size_t size() const noexcept;
        const ft_vector<uint8_t>& data() const noexcept;
        size_t tell() const noexcept;
        bool seek(size_t position) noexcept;

        explicit operator bool() const noexcept;
        bool good() const noexcept;
        bool bad() const noexcept;

        template <typename ValueType>
        data_buffer_proxy operator<<(const ValueType &value) noexcept;

        template <typename ValueType>
        data_buffer_proxy operator>>(ValueType &value) noexcept;

        int enable_thread_safety(void) noexcept;
        int disable_thread_safety(void) noexcept;
        bool is_thread_safe(void) const noexcept;

        static int32_t last_operation_error() noexcept;
        static const char *last_operation_error_str() noexcept;
        int32_t get_operation_error() const noexcept;
        const char *get_operation_error_str() const noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

template <typename ValueType>
int DataBuffer::write_value(const ValueType &value) noexcept
{
    std::ostringstream output_stream;
    output_stream << value;
    char *bytes = adv_strdup(output_stream.str().c_str());
    if (bytes == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    size_t length = ft_strlen_size_t(bytes);
    int length_error = this->write_length_locked(length);
    if (length_error != FT_ERR_SUCCESS)
    {
        cma_free(bytes);
        return (length_error);
    }
    size_t index = 0;
    while (index < length)
    {
        this->_buffer.push_back(static_cast<uint8_t>(bytes[index]));
        int push_error = ft_vector<uint8_t>::last_operation_error();
        if (push_error != FT_ERR_SUCCESS)
        {
            cma_free(bytes);
            return (push_error);
        }
        index++;
    }
    cma_free(bytes);
    return (FT_ERR_SUCCESS);
}

template <typename ValueType>
int DataBuffer::read_value(ValueType &value) noexcept
{
    size_t length = 0;
    int length_error = this->read_length_locked(length);
    if (length_error != FT_ERR_SUCCESS)
        return (length_error);
    if (this->_read_pos + length > this->_buffer.size())
        return (FT_ERR_INVALID_ARGUMENT);
    char *bytes = static_cast<char *>(adv_calloc(length + 1, sizeof(char)));
    if (bytes == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    ft_memcpy(bytes, this->_buffer.begin() + this->_read_pos, length);
    this->_read_pos += length;
    ft_string string_value(bytes);
    cma_free(bytes);
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        return (ft_string::last_operation_error());
    ft_istringstream input_stream(string_value);
    input_stream >> value;
    return (FT_ERR_SUCCESS);
}

template <typename ValueType>
data_buffer_proxy DataBuffer::operator<<(const ValueType &value) noexcept
{
    this->abort_if_not_initialized("DataBuffer::operator<<");
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_operation_error(lock_error);
        return (data_buffer_proxy(this, lock_error));
    }
    int write_error = this->write_value(value);
    int unlock_error = this->unlock_mutex();
    int final_error = write_error;
    if (unlock_error != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
        final_error = unlock_error;
    this->set_operation_error(final_error);
    return (data_buffer_proxy(this, final_error));
}

template <typename ValueType>
data_buffer_proxy DataBuffer::operator>>(ValueType &value) noexcept
{
    this->abort_if_not_initialized("DataBuffer::operator>>");
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_operation_error(lock_error);
        return (data_buffer_proxy(this, lock_error));
    }
    int read_error = this->read_value(value);
    int unlock_error = this->unlock_mutex();
    int final_error = read_error;
    if (unlock_error != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
        final_error = unlock_error;
    this->set_operation_error(final_error);
    return (data_buffer_proxy(this, final_error));
}

template <typename ValueType>
data_buffer_proxy data_buffer_proxy::operator<<(const ValueType &value) const noexcept
{
    if (this->_error_code != FT_ERR_SUCCESS)
    {
        DataBuffer::set_last_operation_error(this->_error_code);
        return (data_buffer_proxy(this->_data_buffer, this->_error_code));
    }
    if (this->_data_buffer == ft_nullptr)
    {
        DataBuffer::set_last_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (data_buffer_proxy(this->_data_buffer, FT_ERR_INVALID_ARGUMENT));
    }
    return (this->_data_buffer->operator<<(value));
}

template <typename ValueType>
data_buffer_proxy data_buffer_proxy::operator>>(ValueType &value) const noexcept
{
    if (this->_error_code != FT_ERR_SUCCESS)
    {
        DataBuffer::set_last_operation_error(this->_error_code);
        return (data_buffer_proxy(this->_data_buffer, this->_error_code));
    }
    if (this->_data_buffer == ft_nullptr)
    {
        DataBuffer::set_last_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (data_buffer_proxy(this->_data_buffer, FT_ERR_INVALID_ARGUMENT));
    }
    return (this->_data_buffer->operator>>(value));
}

#endif
