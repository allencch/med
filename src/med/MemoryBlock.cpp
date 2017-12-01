// #include "med/MemoryBlock.hpp"
#include "med/MedCommon.hpp"

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

void MemoryBlock::dumpByAddress(MemAddr address, int size, FILE* stream) {
  MemAddr start = getAddress();
  MemAddr end = start + getSize();
  if (address < start || address + size >= end) {
    return;
  }
  Byte* ptr = getData();
  long offset = address - start;
  printHex(stream, ptr + offset, size);
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

void MemoryBlocks::dumpByAddress(MemAddr address, int size, FILE* stream) {
  MemoryBlock* block = getMemoryBlockByAddress(address);
  block->dumpByAddress(address, size, stream);
}

MemoryBlock* MemoryBlocks::getMemoryBlockByAddress(MemAddr address) {
  for (size_t i = 0; i < data.size(); i++) {
    MemAddr start = data[i].getAddress();
    MemAddr end = start + data[i].getSize();
    if (address < start || address >= end) {
      continue;
    }
    return &(data[i]);
  }
  return NULL;
}
