#ifndef SIZED_BYTES_HPP
#define SIZED_BYTES_HPP

#include "med/MedTypes.hpp"

class SizedBytes {
public:
    SizedBytes();
    SizedBytes(const Byte* bytes, size_t length);
    SizedBytes(BytePtr data, size_t length);

    static SizedBytes create(size_t length);

    size_t getSize() const;
    BytePtr getBytePtr() const;
    Byte* getBytes() const;

    bool isEmpty() const;

private:
    std::pair<BytePtr, size_t> data_;
};

#endif
