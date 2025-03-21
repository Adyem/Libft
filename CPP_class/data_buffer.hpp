#ifndef DATA_BUFFER
# define DATA_BUFFER

#include <vector>
#include <cstdint>
#include <sstream>
#include <cstring>

class DataBuffer {
private:
    std::vector<uint8_t> buffer_;
    size_t readPos_;
    bool ok_;

public:
    DataBuffer();
    void clear() noexcept;
    size_t size() const noexcept;
    const std::vector<uint8_t>& data() const noexcept;

    explicit operator bool() const noexcept { return ok_; }
    bool good() const noexcept { return ok_; }
    bool bad() const noexcept { return !ok_; }

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
    buffer_.insert(buffer_.end(), bytes.begin(), bytes.end());
    return *this;
}

template<typename T>
DataBuffer& DataBuffer::operator>>(T& value) {
    size_t len;
    *this >> len;
    if (!ok_ || readPos_ + len > buffer_.size()) {
        ok_ = false;
        return *this;
    }
    std::string bytes(reinterpret_cast<const char*>(buffer_.data() + readPos_), len);
    std::istringstream iss(bytes);
    iss >> value;
    ok_ = !iss.fail();
    readPos_ += len;
    return *this;
}

#endif
