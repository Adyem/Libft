#ifndef DATA_BUFFER
# define DATA_BUFFER

#include "../Template/vector.hpp"
#include <cstdint>
#include <sstream>
#include "class_istringstream.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class DataBuffer
{
    private:
        ft_vector<uint8_t> _buffer;
        size_t _read_pos;
        bool _ok;
        mutable int _error_code;
        mutable pt_mutex _mutex;

        void set_error_unlocked(int error_code) const noexcept;
        void set_error(int error_code) const noexcept;
        int lock_self(ft_unique_lock<pt_mutex> &guard) const noexcept;
        static int lock_pair(const DataBuffer &first, const DataBuffer &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard) noexcept;
        static void sleep_backoff() noexcept;
        static void restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno) noexcept;
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
        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

template<typename T>
DataBuffer& DataBuffer::operator<<(const T& value)
{
    std::ostringstream oss;
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    char *bytes;
    size_t len;
    int length_error;
    size_t index;

    oss << value;
    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (*this);
    }
    bytes = cma_strdup(oss.str().c_str());
    if (!bytes)
    {
        this->_ok = false;
        this->set_error_unlocked(FT_ERR_NO_MEMORY);
        DataBuffer::restore_errno(guard, entry_errno);
        return (*this);
    }
    len = ft_strlen_size_t(bytes);
    length_error = this->write_length_locked(len);
    if (length_error != FT_ER_SUCCESSS)
    {
        this->_ok = false;
        this->set_error_unlocked(length_error);
        cma_free(bytes);
        DataBuffer::restore_errno(guard, entry_errno);
        return (*this);
    }
    index = 0;
    while (index < len)
    {
        this->_buffer.push_back(static_cast<uint8_t>(bytes[index]));
        if (this->_buffer.get_error() != FT_ER_SUCCESSS)
        {
            this->_ok = false;
            this->set_error_unlocked(this->_buffer.get_error());
            cma_free(bytes);
            DataBuffer::restore_errno(guard, entry_errno);
            return (*this);
        }
        ++index;
    }
    cma_free(bytes);
    this->_ok = true;
    this->set_error_unlocked(FT_ER_SUCCESSS);
    DataBuffer::restore_errno(guard, entry_errno);
    return (*this);
}

template<typename T>
DataBuffer& DataBuffer::operator>>(T& value)
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    size_t len;
    int length_error;
    char *bytes;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (*this);
    }
    length_error = this->read_length_locked(len);
    if (length_error != FT_ER_SUCCESSS)
    {
        this->_ok = false;
        this->set_error_unlocked(length_error);
        DataBuffer::restore_errno(guard, entry_errno);
        return (*this);
    }
    if (this->_read_pos + len > this->_buffer.size())
    {
        this->_ok = false;
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        DataBuffer::restore_errno(guard, entry_errno);
        return (*this);
    }
    bytes = static_cast<char*>(cma_calloc(len + 1, sizeof(char)));
    if (!bytes)
    {
        this->_ok = false;
        this->set_error_unlocked(FT_ERR_NO_MEMORY);
        DataBuffer::restore_errno(guard, entry_errno);
        return (*this);
    }
    ft_memcpy(bytes, this->_buffer.begin() + this->_read_pos, len);
    this->_read_pos += len;
    ft_string string_value(bytes);
    ft_istringstream iss(string_value);
    iss >> value;
    cma_free(bytes);
    this->_ok = (iss.get_error() == FT_ER_SUCCESSS);
    if (this->_ok)
        this->set_error_unlocked(FT_ER_SUCCESSS);
    else
        this->set_error_unlocked(iss.get_error());
    DataBuffer::restore_errno(guard, entry_errno);
    return (*this);
}

#endif
