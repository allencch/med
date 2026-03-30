#include <cstring>
#include "med/SizedBytes.hpp"

SizedBytes::SizedBytes() {
  BytePtr data(NULL);
  this->data = std::make_pair(data, 0);
}

SizedBytes::SizedBytes(Byte* bytes, int length) {
  Byte* newBytes = new Byte[length];
  memcpy(newBytes, bytes, length);
  BytePtr data(newBytes);
  this->data = std::make_pair(data, length);
}

SizedBytes::SizedBytes(BytePtr data, int length) {
  this->data = std::make_pair(data, length);
}

SizedBytes SizedBytes::create(int length) {
  BytePtr bytePtr(new Byte[length]);
  return SizedBytes(bytePtr, length);
}

size_t SizedBytes::getSize() {
  return std::get<1>(data);
}

BytePtr SizedBytes::getBytePtr() {
  return std::get<0>(data);
}

Byte* SizedBytes::getBytes() {
  auto bytePtr = getBytePtr();
  return bytePtr.get();
}

bool SizedBytes::isEmpty() {
  return getSize() == 0;
}
