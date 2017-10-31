#ifndef BYTES_HPP
#define BYTES_HPP

#include "med/MedTypes.hpp"

class Bytes {
public:
  Bytes();
  Bytes(Byte* data, int size);

  Byte* data;
  int size;

  virtual void setData(Byte* data, int size);
  void free();
};

#endif
