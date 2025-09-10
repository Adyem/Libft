#include "class_data_buffer.hpp"
#include "../Libft/libft.hpp"
#include <utility>

DataBuffer::DataBuffer() : _readPos(0), _ok(true)
{
    return ;
}

DataBuffer::DataBuffer(const DataBuffer& other)
    : _buffer(other._buffer), _readPos(other._readPos), _ok(other._ok)
{
    return ;
}

DataBuffer::DataBuffer(DataBuffer&& other) noexcept
    : _buffer(std::move(other._buffer)), _readPos(other._readPos), _ok(other._ok)
{
    other._readPos = 0;
    other._ok = true;
    return ;
}

DataBuffer& DataBuffer::operator=(const DataBuffer& other)
{
    if (this != &other)
    {
        this->_buffer = other._buffer;
        this->_readPos = other._readPos;
        this->_ok = other._ok;
    }
    return (*this);
}

DataBuffer& DataBuffer::operator=(DataBuffer&& other) noexcept
{
    if (this != &other)
    {
        this->_buffer = std::move(other._buffer);
        this->_readPos = other._readPos;
        this->_ok = other._ok;
        other._readPos = 0;
        other._ok = true;
    }
    return (*this);
}

DataBuffer::~DataBuffer()
{
    return ;
}

void DataBuffer::clear() noexcept
{
    this->_buffer.clear();
    this->_readPos = 0;
    this->_ok = true;
    return ;
}

size_t DataBuffer::size() const noexcept
{
    return (this->_buffer.size());
}

const std::vector<uint8_t>& DataBuffer::data() const noexcept
{
    return (this->_buffer);
}

size_t DataBuffer::tell() const noexcept
{
    return (this->_readPos);
}

bool DataBuffer::seek(size_t pos) noexcept
{
    if (pos <= this->_buffer.size())
    {
        this->_readPos = pos;
        this->_ok = true;
        return (true);
    }
    this->_ok = false;
    return (false);
}

DataBuffer::operator bool() const noexcept
{
    return (this->_ok);
}

bool DataBuffer::good() const noexcept
{
    return (this->_ok);
}

bool DataBuffer::bad() const noexcept
{
    return (!this->_ok);
}

DataBuffer& DataBuffer::operator<<(size_t len)
{
    auto ptr = reinterpret_cast<const uint8_t*>(&len);
    this->_buffer.insert(this->_buffer.end(), ptr, ptr + sizeof(size_t));
    return (*this);
}

DataBuffer& DataBuffer::operator>>(size_t& len)
{
    if (this->_readPos + sizeof(size_t) > this->_buffer.size())
    {
        this->_ok = false;
        return (*this);
    }
    ft_memcpy(&len, this->_buffer.data() + this->_readPos, sizeof(size_t));
    this->_readPos += sizeof(size_t);
    this->_ok = true;
    return (*this);
}
