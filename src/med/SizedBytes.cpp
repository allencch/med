#include "med/SizedBytes.hpp"

SizedBytes::SizedBytes() {}
SizedBytes::SizedBytes(BytePtr data, int length) {
  this->data = std::make_pair(data, length);
}

size_t SizedBytes::getSize() {
  return std::get<1>(data);
}

BytePtr SizedBytes::getBytePtr() {
  return std::get<0>(data);
}

Byte* SizedBytes::getByte() {
  auto bytePtr = getBytePtr();
  return bytePtr.get();
}
