#include "med/MemoryBlock.hpp"

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
