#ifndef DATA_BUFFER
# define DATA_BUFFER

#include "../Template/vector.hpp"
#include <cstdint>
#include <sstream>
#include "class_istringstream.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"

class DataBuffer
{
    private:
        ft_vector<uint8_t> _buffer;
        size_t _read_pos;
        bool _ok;
        mutable int _error_code;

        void set_error(int error_code) const noexcept;

    public:
        DataBuffer();
        DataBuffer(const DataBuffer& other);
        DataBuffer(DataBuffer&& other) noexcept;
        DataBuffer& operator=(const DataBuffer& other);
        DataBuffer& operator=(DataBuffer&& other) noexcept;
        ~DataBuffer();
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
    oss << value;
    char *bytes = cma_strdup(oss.str().c_str());
    if (!bytes)
    {
        this->_ok = false;
        this->set_error(CMA_BAD_ALLOC);
        return (*this);
    }
    size_t len = ft_strlen_size_t(bytes);
    *this << len;
    if (!this->_ok)
    {
        cma_free(bytes);
        return (*this);
    }
    size_t index = 0;
    while (index < len)
    {
        this->_buffer.push_back(static_cast<uint8_t>(bytes[index]));
        if (this->_buffer.get_error() != ER_SUCCESS)
        {
            this->_ok = false;
            this->set_error(this->_buffer.get_error());
            cma_free(bytes);
            return (*this);
        }
        ++index;
    }
    cma_free(bytes);
    this->_ok = true;
    this->set_error(ER_SUCCESS);
    return (*this);
}

template<typename T>
DataBuffer& DataBuffer::operator>>(T& value)
{
    size_t len;
    *this >> len;
    if (!this->_ok)
        return (*this);
    if (!this->_ok || this->_read_pos + len > this->_buffer.size())
    {
        this->_ok = false;
        this->set_error(FT_EINVAL);
        return (*this);
    }
    char *bytes = static_cast<char*>(cma_calloc(len + 1, sizeof(char)));
    if (!bytes)
    {
        this->_ok = false;
        this->set_error(CMA_BAD_ALLOC);
        return (*this);
    }
    ft_memcpy(bytes, this->_buffer.begin() + this->_read_pos, len);
    ft_istringstream iss(bytes);
    iss >> value;
    cma_free(bytes);
    this->_ok = (iss.get_error() == ER_SUCCESS);
    if (this->_ok)
        this->set_error(ER_SUCCESS);
    else
        this->set_error(iss.get_error());
    this->_read_pos += len;
    return (*this);
}

#endif
