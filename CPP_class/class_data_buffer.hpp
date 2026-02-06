#ifndef DATA_BUFFER
# define DATA_BUFFER

#include "../Template/vector.hpp"
#include <cstdint>
#include <sstream>
#include "class_istringstream.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"

class DataBuffer
{
    private:
        ft_vector<uint8_t> _buffer;
        size_t _read_pos;
        bool _ok;
        mutable pt_recursive_mutex *_mutex;

        int lock_mutex(void) const noexcept;
        int unlock_mutex(void) const noexcept;
        int prepare_thread_safety(void) noexcept;
        void teardown_thread_safety(void) noexcept;
        int enable_thread_safety(void) noexcept;
        void disable_thread_safety(void) noexcept;
        bool is_thread_safe_enabled(void) const noexcept;
        static int lock_pair(const DataBuffer &first, const DataBuffer &second,
                const DataBuffer *&lower, const DataBuffer *&upper) noexcept;
        static int unlock_pair(const DataBuffer *lower, const DataBuffer *upper) noexcept;
        static void sleep_backoff() noexcept;
        int write_length_locked(size_t len) noexcept;
        int read_length_locked(size_t &len) noexcept;
    public:
        DataBuffer() noexcept;
        DataBuffer(const DataBuffer& other) noexcept;
        DataBuffer(DataBuffer&& other) noexcept;
        DataBuffer& operator=(const DataBuffer& other) noexcept;
        DataBuffer& operator=(DataBuffer&& other) noexcept;
        ~DataBuffer() noexcept;
        void clear() noexcept;
        size_t size() const noexcept;
        const ft_vector<uint8_t>& data() const noexcept;
        size_t tell() const noexcept;
        bool seek(size_t pos) noexcept;

        explicit operator bool() const noexcept;
        bool good() const noexcept;
        bool bad() const noexcept;

        template<typename T>
        DataBuffer& operator<<(const T& value);

        template<typename T>
        DataBuffer& operator>>(T& value);

        DataBuffer& operator<<(size_t len);
        DataBuffer& operator>>(size_t& len);
#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

template<typename T>
DataBuffer& DataBuffer::operator<<(const T& value)
{
    std::ostringstream oss;
    oss << value;
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    int final_error = FT_ERR_SUCCESSS;
    char *bytes = cma_strdup(oss.str().c_str());
    if (bytes == ft_nullptr)
    {
        this->_ok = false;
        final_error = FT_ERR_NO_MEMORY;
    }
    else
    {
        size_t len = ft_strlen_size_t(bytes);
        int length_error = this->write_length_locked(len);
        if (length_error != FT_ERR_SUCCESSS)
        {
            this->_ok = false;
            final_error = length_error;
        }
        else
        {
            size_t index = 0;
            while (index < len)
            {
                this->_buffer.push_back(static_cast<uint8_t>(bytes[index]));
                int buffer_error = ft_global_error_stack_peek_last_error();
                if (buffer_error != FT_ERR_SUCCESSS)
                {
                    this->_ok = false;
                    final_error = buffer_error;
                    break ;
                }
                ++index;
            }
        }
        cma_free(bytes);
    }
    this->_ok = (final_error == FT_ERR_SUCCESSS);
    {
        int unlock_error = this->unlock_mutex();
        if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
            final_error = unlock_error;
    }
    ft_global_error_stack_push(final_error);
    return (*this);
}

template<typename T>
DataBuffer& DataBuffer::operator>>(T& value)
{
    int lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    int final_error = FT_ERR_SUCCESSS;
    size_t len = 0;
    char *bytes = ft_nullptr;
    int length_error = this->read_length_locked(len);
    if (length_error != FT_ERR_SUCCESSS)
    {
        final_error = length_error;
    }
    else if (this->_read_pos + len > this->_buffer.size())
    {
        final_error = FT_ERR_INVALID_ARGUMENT;
    }
    else
    {
        bytes = static_cast<char*>(cma_calloc(len + 1, sizeof(char)));
        if (bytes == ft_nullptr)
        {
            final_error = FT_ERR_NO_MEMORY;
        }
        else
        {
            ft_memcpy(bytes, this->_buffer.begin() + this->_read_pos, len);
            this->_read_pos += len;
            ft_string string_value(bytes);
            ft_istringstream iss(string_value);
            iss >> value;
            final_error = ft_global_error_stack_peek_last_error();
            cma_free(bytes);
        }
    }
    this->_ok = (final_error == FT_ERR_SUCCESSS);
    {
        int unlock_error = this->unlock_mutex();
        if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
            final_error = unlock_error;
    }
    ft_global_error_stack_push(final_error);
    return (*this);
}

#endif
