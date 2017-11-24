#include "med/MemoryBlock.hpp"

//// MemoryBlock

MemoryBlock::MemoryBlock() : Bytes() {}

MemoryBlock::MemoryBlock(Byte* data, int size) : Bytes(data, size) {}

MemoryBlock::~MemoryBlock() {
  // Do not free Byte
}

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
  free(); // Will call MemoryBlock.free(), which will delete Byte
  data.clear(); // This will clear the vector of MemoryBlock
}

int MemoryBlocks::getSize() {
  return data.size();
}

vector<MemoryBlock> MemoryBlocks::getData() {
  return data;
}
