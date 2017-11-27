#ifndef BYTES_HPP
#define BYTES_HPP

#include "med/MedTypes.hpp"

class Bytes {
public:
  Bytes();
  Bytes(Byte* data, int size);
  virtual ~Bytes();

  Byte* data;
  int size;

  virtual void setData(Byte* data, int size);
  Byte* getData();
  void free();
  int getSize();

  void dump(FILE* stream = stdout);

  static Bytes copy(Byte* data, int size);
  static Bytes* newCopy(Byte* data, int size);

  static Bytes* create(int size, Byte value = 0);
};

#endif
