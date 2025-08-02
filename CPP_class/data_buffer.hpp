#ifndef DATA_BUFFER
# define DATA_BUFFER

#include <vector>
#include <cstdint>
#include <sstream>
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"

class DataBuffer {
private:
    std::vector<uint8_t> _buffer;
    size_t _readPos;
    bool _ok;

public:
    DataBuffer();
    void clear() noexcept;
    size_t size() const noexcept;
    const std::vector<uint8_t>& data() const noexcept;
    size_t tell() const noexcept;
    bool seek(size_t pos) noexcept;

    explicit operator bool() const noexcept { return (this->_ok); }
    bool good() const noexcept { return (this->_ok); }
    bool bad() const noexcept { return (!this->_ok); }

    template<typename T>
    DataBuffer& operator<<(const T& value);

    template<typename T>
    DataBuffer& operator>>(T& value);

    DataBuffer& operator<<(size_t len);
    DataBuffer& operator>>(size_t& len);
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
        ft_errno = CMA_BAD_ALLOC;
        return (*this);
    }
    size_t len = ft_strlen_size_t(bytes);
    *this << len;
    this->_buffer.insert(this->_buffer.end(), bytes, bytes + len);
    cma_free(bytes);
    return (*this);
}

template<typename T>
DataBuffer& DataBuffer::operator>>(T& value)
{
    size_t len;
    *this >> len;
    if (!this->_ok || this->_readPos + len > this->_buffer.size())
	{
        this->_ok = false;
        return (*this);
    }
    char *bytes = static_cast<char*>(cma_calloc(len + 1, sizeof(char)));
    if (!bytes)
	{
        this->_ok = false;
        ft_errno = CMA_BAD_ALLOC;
        return (*this);
    }
    ft_memcpy(bytes, this->_buffer.data() + this->_readPos, len);
    std::istringstream iss(bytes);
    iss >> value;
    cma_free(bytes);
    this->_ok = !iss.fail();
    this->_readPos += len;
    return (*this);
}

#endif
