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


//// MemoryBlock

MemoryBlock::MemoryBlock() : Bytes() {}

MemoryBlock::MemoryBlock(Byte* data, int size) : Bytes(data, size) {}

void MemoryBlock::setAddress(MemAddr address) {
  this->address = address;
}

MemAddr MemoryBlock::getAddress() {
  return address;
}

void MemoryBlock::setDataWithAddress(Byte* data, int size, MemAddr address) {
  setData(data, size);
  setAddress(address);
}


/// MemoryBlocks

MemoryBlocks::MemoryBlocks() {}

void MemoryBlocks::free() {
  for (size_t i = 0; i < data.size(); i++) {
    data[i].free();
  }
}

void MemoryBlocks::push(MemoryBlock block) {
  data.push_back(block);
}

void MemoryBlocks::clear() {
  free();
  data.clear();
}

int MemoryBlocks::getSize() {
  return data.size();
}

vector<MemoryBlock> MemoryBlocks::getData() {
  return data;
}
