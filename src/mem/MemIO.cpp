#include <cstring>
#include <string>
#include <unistd.h> //open, read, lseek

#include "med/MedException.hpp"
#include "med/MedCommon.hpp"
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
  if (pid) {
    return readProcess(addr, size);
  }
  return readDirect(addr, size);
}

MemPtr MemIO::readDirect(Address addr, size_t size) {
  MemPtr mem = MemPtr(new Mem(addr, size));
  return mem;
}

MemPtr MemIO::readProcess(Address addr, size_t size) {
  mutex.lock();
  pidAttach(pid);

  MemPtr mem = MemPtr(new Mem(size));

  int memFd = getMem(pid);
  if(lseek(memFd, addr, SEEK_SET) == -1) {
    close(memFd);
    pidDetach(pid);
    mutex.unlock();
    throw MedException("Failed lseek");
  }
  if(::read(memFd, mem->data, size) == -1) {
    close(memFd);
    pidDetach(pid);
    mutex.unlock();
    throw MedException("Address read fail: " + intToHex(addr));
  }

  close(memFd);
  pidDetach(pid);
  mutex.unlock();
  return mem;
}

void MemIO::write(Address addr, MemPtr mem, size_t size) {
  if (pid) {
    return writeProcess(addr, mem, size);
  }
  return writeDirect(addr, mem, size);
}

void MemIO::writeDirect(Address addr, MemPtr mem, size_t size) {
  Byte* ptr = (Byte*)addr;
  int writeSize = size ? size : mem->size;
  memcpy(ptr, mem->data, writeSize);
}

void MemIO::writeProcess(Address addr, MemPtr mem, size_t size) {

}
