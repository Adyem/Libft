#include "data_buffer.hpp"

DataBuffer::DataBuffer() : readPos_(0), ok_(true) {}

void DataBuffer::clear() noexcept {
    this->buffer_.clear();
    this->readPos_ = 0;
    this->ok_ = true;
}

size_t DataBuffer::size() const noexcept {
    return (this->buffer_.size());
}

const std::vector<uint8_t>& DataBuffer::data() const noexcept {
    return buffer_;
}

DataBuffer& DataBuffer::operator<<(size_t len) {
    auto ptr = reinterpret_cast<const uint8_t*>(&len);
    buffer_.insert(buffer_.end(), ptr, ptr + sizeof(size_t));
    return *this;
}

DataBuffer& DataBuffer::operator>>(size_t& len) {
    if (readPos_ + sizeof(size_t) > buffer_.size()) {
        ok_ = false;
        return *this;
    }
    std::memcpy(&len, buffer_.data() + readPos_, sizeof(size_t));
    readPos_ += sizeof(size_t);
    ok_ = true;
    return *this;
}
