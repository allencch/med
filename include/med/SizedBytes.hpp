#ifndef SIZED_BYTES_HPP
#define SIZED_BYTES_HPP

#include "med/MedTypes.hpp"

class SizedBytes {
public:
  SizedBytes();
  SizedBytes(BytePtr data, int length);
  size_t getSize();
  BytePtr getBytePtr();
  Byte* getByte();

private:
  pair<BytePtr, size_t> data;
};
#endif
