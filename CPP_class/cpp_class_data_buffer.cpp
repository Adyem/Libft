#include "class_data_buffer.hpp"
#include "../Libft/libft.hpp"
#include "../Template/move.hpp"

DataBuffer::DataBuffer() : _read_pos(0), _ok(true)
{
    return ;
}

DataBuffer::DataBuffer(const DataBuffer& other)
    : _buffer(other._buffer.size()), _read_pos(other._read_pos), _ok(other._ok)
{
    size_t index = 0;
    size_t other_size = other._buffer.size();
    while (index < other_size)
    {
        this->_buffer.push_back(other._buffer[index]);
        if (this->_buffer.get_error() != ER_SUCCESS)
        {
            this->_ok = false;
            return ;
        }
        ++index;
    }
    return ;
}

DataBuffer::DataBuffer(DataBuffer&& other) noexcept
    : _buffer(ft_move(other._buffer)), _read_pos(other._read_pos), _ok(other._ok)
{
    other._read_pos = 0;
    other._ok = true;
    return ;
}

DataBuffer& DataBuffer::operator=(const DataBuffer& other)
{
    if (this != &other)
    {
        this->_buffer.clear();
        size_t index = 0;
        size_t other_size = other._buffer.size();
        while (index < other_size)
        {
            this->_buffer.push_back(other._buffer[index]);
            if (this->_buffer.get_error() != ER_SUCCESS)
            {
                this->_ok = false;
                return (*this);
            }
            ++index;
        }
        this->_read_pos = other._read_pos;
        this->_ok = other._ok;
    }
    return (*this);
}

DataBuffer& DataBuffer::operator=(DataBuffer&& other) noexcept
{
    if (this != &other)
    {
        this->_buffer = ft_move(other._buffer);
        this->_read_pos = other._read_pos;
        this->_ok = other._ok;
        other._read_pos = 0;
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
    this->_read_pos = 0;
    this->_ok = true;
    return ;
}

size_t DataBuffer::size() const noexcept
{
    return (this->_buffer.size());
}

const ft_vector<uint8_t>& DataBuffer::data() const noexcept
{
    return (this->_buffer);
}

size_t DataBuffer::tell() const noexcept
{
    return (this->_read_pos);
}

bool DataBuffer::seek(size_t pos) noexcept
{
    if (pos <= this->_buffer.size())
    {
        this->_read_pos = pos;
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
    size_t index = 0;
    while (index < sizeof(size_t))
    {
        this->_buffer.push_back(ptr[index]);
        if (this->_buffer.get_error() != ER_SUCCESS)
        {
            this->_ok = false;
            return (*this);
        }
        ++index;
    }
    return (*this);
}

DataBuffer& DataBuffer::operator>>(size_t& len)
{
    if (this->_read_pos + sizeof(size_t) > this->_buffer.size())
    {
        this->_ok = false;
        return (*this);
    }
    ft_memcpy(&len, this->_buffer.begin() + this->_read_pos, sizeof(size_t));
    this->_read_pos += sizeof(size_t);
    this->_ok = true;
    return (*this);
}
