#include <cstring>
#include "mem/MemIO.hpp"

MemIO::MemIO() {
  pid = 0;
}

void MemIO::setPid(pid_t pid) {
  this->pid = pid;
}

pid_t MemIO::getPid() {
  return pid;
}

MemPtr MemIO::read(Address addr, size_t size) {
  MemPtr mem = MemPtr(new Mem(addr, size));
  return mem;
}

void MemIO::write(Address addr, MemPtr mem, size_t size) {
  Byte* ptr = (Byte*)addr;
  int writeSize = size ? size : mem->size;
  memcpy(ptr, mem->data, writeSize);
}
