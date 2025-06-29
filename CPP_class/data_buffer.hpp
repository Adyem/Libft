#ifndef DATA_BUFFER
# define DATA_BUFFER

#include <vector>
#include <cstdint>
#include <sstream>
#include <cstring>

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
DataBuffer& DataBuffer::operator<<(const T& value) {
    std::ostringstream oss;
    oss << value;
    std::string bytes = oss.str();
    *this << bytes.size();
    this->_buffer.insert(this->_buffer.end(), bytes.begin(), bytes.end());
    return (*this);
}

template<typename T>
DataBuffer& DataBuffer::operator>>(T& value) {
    size_t len;
    *this >> len;
    if (!this->_ok || this->_readPos + len > this->_buffer.size()) {
        this->_ok = false;
        return (*this);
    }
    std::string bytes(reinterpret_cast<const char*>(this->_buffer.data() + this->_readPos), len);
    std::istringstream iss(bytes);
    iss >> value;
    this->_ok = !iss.fail();
    this->_readPos += len;
    return (*this);
}

#endif
