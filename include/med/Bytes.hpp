#ifndef BYTES_HPP
#define BYTES_HPP

#include "med/MedTypes.hpp"

class Bytes {
public:
  Bytes(Byte* data, int size);
  Byte* data;
  int size;
};

#endif
