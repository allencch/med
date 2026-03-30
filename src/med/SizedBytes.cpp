#include <cstring>
#include "med/SizedBytes.hpp"

SizedBytes::SizedBytes() {
    data_ = std::make_pair(nullptr, 0);
}

SizedBytes::SizedBytes(Byte* bytes, size_t length) {
    if (length == 0) {
        data_ = std::make_pair(nullptr, 0);
    } else {
        BytePtr ptr(new Byte[length]);
        std::memcpy(ptr.get(), bytes, length);
        data_ = std::make_pair(ptr, length);
    }
}

SizedBytes::SizedBytes(BytePtr data, size_t length) {
    data_ = std::make_pair(data, length);
}

SizedBytes SizedBytes::create(size_t length) {
    if (length == 0) return SizedBytes();
    BytePtr bytePtr(new Byte[length]);
    return SizedBytes(bytePtr, length);
}

size_t SizedBytes::getSize() const {
    return data_.second;
}

BytePtr SizedBytes::getBytePtr() const {
    return data_.first;
}

Byte* SizedBytes::getBytes() const {
    return data_.first.get();
}

bool SizedBytes::isEmpty() const {
    return getSize() == 0;
}
