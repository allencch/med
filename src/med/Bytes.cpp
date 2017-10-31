#include <iostream>
#include <cstdio>

#include "med/Bytes.hpp"

using namespace std;

Bytes::Bytes() {
  data = NULL;
  size = 0;
}

Bytes::Bytes(Byte* data, int size) : data(data), size(size) {}

void Bytes::setData(Byte* data, int size) {
  this->data = data;
  this->size = size;
}

void Bytes::free() {
  if (data) {
    delete[] data;
    data = NULL;
    size = 0;
  }
}
