#include <iostream>
#include <cstdio>
#include <cstring>

#include "med/Bytes.hpp"

using namespace std;

Bytes::Bytes() {
  data = NULL;
  size = 0;
}

Bytes::Bytes(Byte* data, int size) : data(data), size(size) {}

Bytes::~Bytes() {
  // Do not free data directly!!! Because the data is not allocated by its own, but assigned!
}

void Bytes::setData(Byte* data, int size) {
  if (size > 0) {
    this->data = data;
    this->size = size;
  }
}

void Bytes::free() {
  if (size > 0 && data) {
    delete[] data;
    data = NULL;
    size = 0;
  }
}

int Bytes::getSize() {
  return size;
}

Byte* Bytes::getData() {
  return data;
}

void Bytes::dump(FILE* stream) {
  for (int i = 0; i < size; i++) {
    fprintf(stream, "%02x ", (unsigned int)data[i]);
  }
  fprintf(stream, "\n");
}

Bytes Bytes::copy(Byte* data, int size) {
  Byte* newData = new Byte[size];
  memcpy(newData, data, size);
  return Bytes(data, size);
}

Bytes* Bytes::newCopy(Byte* data, int size) {
  Byte* newData = new Byte[size];
  memcpy(newData, data, size);
  return new Bytes(data, size);
}
