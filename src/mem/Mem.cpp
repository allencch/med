#include <cstring>
#include <cstdio>
#include "mem/Mem.hpp"

Mem::Mem(size_t size) {
  initialize(size);
}

Mem::Mem(Address addr, size_t size) {
  initialize(size);
  memcpy(data, (void*)addr, size);
}

Mem::~Mem() {
  delete[] data;
  size = 0;
}

void Mem::initialize(size_t size) {
  data = new Byte[size];
  this->size = size;
}

void Mem::dump() {
  for(size_t i = 0; i < size ; i++) {
    printf("%x ", data[i]);
  }
  printf("\n");
}
