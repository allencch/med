#include <cstring>
#include <cstdio>
#include <iostream>
#include "mem/Mem.hpp"
#include "med/MedCommon.hpp"

Mem::Mem(size_t size) {
  initialize(size);
}

Mem::Mem(Address addr, size_t size) {
  initialize(size);
  memcpy(data, (void*)addr, size);
  address = addr;
}

Mem::~Mem() {
  delete[] data;
  size = 0;
}

void Mem::initialize(size_t size) {
  data = new Byte[size];
  this->size = size;
  address = 0;
}

void Mem::dump(bool newline) {
  for (size_t i = 0; i < size ; i++) {
    printf("%x ", data[i]);
  }
  if (newline) printf("\n");
}

void Mem::setValue(int value) {
  Byte* ptr = (Byte*)&value;
  memcpy(data, ptr, size);
}

int Mem::getValueAsInt() {
  int value = 0;
  Byte* ptr = (Byte*)&value;
  memcpy(ptr, data, size);
  return value;
}

Address Mem::getAddress() {
  return address;
}

string Mem::getAddressAsString() {
  return intToHex(getAddress());
}

void Mem::setAddress(Address addr) {
  address = addr;
}
