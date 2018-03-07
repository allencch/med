#ifndef MEM_H
#define MEM_H

#include <memory>
#include "med/MedTypes.hpp"

class Mem {
public:
  explicit Mem(size_t size);
  Mem(Address addr, size_t size);
  ~Mem();

  void dump();
  void setValue(int value);

  int getValueAsInt();

  Byte* data;
  size_t size;
private:
  void initialize(size_t size);
};

typedef std::shared_ptr<Mem> MemPtr;

#endif
