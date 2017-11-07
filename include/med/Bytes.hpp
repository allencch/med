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


class MemoryBlock : public Bytes {
public:
  MemoryBlock();
  MemoryBlock(Byte* data, int size);

  void setDataWithAddress(Byte* data, int size, MemAddr address);
  void setAddress(MemAddr address);
  MemAddr getAddress();

private:
  MemAddr address;
};

class MemoryBlocks {
public:
  MemoryBlocks(); // Do not destruct by freeing the memory. Free on demand
  void free();
  void push(MemoryBlock block);
  void clear();
  int getSize();

  vector<MemoryBlock> getData();
private:
  vector<MemoryBlock> data;
};

#endif
