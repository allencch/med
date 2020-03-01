#ifndef SIZED_BYTES_HPP
#define SIZED_BYTES_HPP

#include "med/MedTypes.hpp"

class SizedBytes {
public:
  SizedBytes();
  SizedBytes(Byte* bytes, int length);
  SizedBytes(BytePtr data, int length);

  static SizedBytes create(int length);

  size_t getSize();
  BytePtr getBytePtr();
  Byte* getBytes();

  bool isEmpty();

private:
  pair<BytePtr, size_t> data;
};
#endif
