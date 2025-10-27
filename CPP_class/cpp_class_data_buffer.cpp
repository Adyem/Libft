#include "class_data_buffer.hpp"
#include "../Libft/libft.hpp"
#include <utility>
#include "../Template/move.hpp"

DataBuffer::DataBuffer() noexcept
    : _buffer(), _read_pos(0), _ok(true), _error_code(ER_SUCCESS)
{
    return ;
}

DataBuffer::DataBuffer(const DataBuffer& other) noexcept
    : _buffer(other._buffer.size()), _read_pos(other._read_pos), _ok(other._ok), _error_code(other._error_code)
{
    size_t index = 0;
    size_t other_size = other._buffer.size();
    while (index < other_size)
    {
        this->_buffer.push_back(other._buffer[index]);
        if (this->_buffer.get_error() != ER_SUCCESS)
        {
            this->_ok = false;
            this->set_error(this->_buffer.get_error());
            return ;
        }
        ++index;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

DataBuffer::DataBuffer(DataBuffer&& other) noexcept
    : _buffer(ft_move(other._buffer)), _read_pos(other._read_pos), _ok(other._ok), _error_code(other._error_code)
{
    other._read_pos = 0;
    other._ok = true;
    other._error_code = ER_SUCCESS;
    this->set_error(ER_SUCCESS);
    return ;
}

DataBuffer& DataBuffer::operator=(const DataBuffer& other) noexcept
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
                this->set_error(this->_buffer.get_error());
                return (*this);
            }
            ++index;
        }
        this->_read_pos = other._read_pos;
        this->_ok = other._ok;
        this->_error_code = other._error_code;
        this->set_error(ER_SUCCESS);
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
        this->_error_code = other._error_code;
        other._read_pos = 0;
        other._ok = true;
        other._error_code = ER_SUCCESS;
        this->set_error(ER_SUCCESS);
    }
    return (*this);
}

DataBuffer::~DataBuffer() noexcept
{
    return ;
}

void DataBuffer::clear() noexcept
{
    this->_buffer.clear();
    this->_read_pos = 0;
    this->_ok = true;
    this->set_error(ER_SUCCESS);
    return ;
}

size_t DataBuffer::size() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_buffer.size());
}

const ft_vector<uint8_t>& DataBuffer::data() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_buffer);
}

size_t DataBuffer::tell() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_read_pos);
}

bool DataBuffer::seek(size_t pos) noexcept
{
    if (pos <= this->_buffer.size())
    {
        this->_read_pos = pos;
        this->_ok = true;
        this->set_error(ER_SUCCESS);
        return (true);
    }
    this->_ok = false;
    this->set_error(FT_ERR_INVALID_ARGUMENT);
    return (false);
}

DataBuffer::operator bool() const noexcept
{
    if (this->_ok)
        this->set_error(ER_SUCCESS);
    return (this->_ok);
}

bool DataBuffer::good() const noexcept
{
    if (this->_ok)
        this->set_error(ER_SUCCESS);
    return (this->_ok);
}

bool DataBuffer::bad() const noexcept
{
    if (this->_ok)
    {
        this->set_error(ER_SUCCESS);
        return (false);
    }
    return (true);
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
            this->set_error(this->_buffer.get_error());
            return (*this);
        }
        ++index;
    }
    this->_ok = true;
    this->set_error(ER_SUCCESS);
    return (*this);
}

DataBuffer& DataBuffer::operator>>(size_t& len)
{
    if (this->_read_pos + sizeof(size_t) > this->_buffer.size())
    {
        this->_ok = false;
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (*this);
    }
    ft_memcpy(&len, this->_buffer.begin() + this->_read_pos, sizeof(size_t));
    this->_read_pos += sizeof(size_t);
    this->_ok = true;
    this->set_error(ER_SUCCESS);
    return (*this);
}

void DataBuffer::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

int DataBuffer::get_error() const noexcept
{
    return (this->_error_code);
}

const char *DataBuffer::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}
