#include <cstring>
#include <string>
#include <sys/ptrace.h> //ptrace()
#include <sys/prctl.h> //prctl()
#include <unistd.h> //open, read, lseek

#include "med/MedException.hpp"
#include "med/MedCommon.hpp"
#include "mem/MemIO.hpp"
#include "mem/Pem.hpp"

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

  // When read process, use Pem so that the PemPtr can get data
  // from process through MemIO.
  MemPtr mem = MemPtr(new Pem(size, this));
  mem->setAddress(addr);

  int memFd = getMem(pid);
  if(lseek(memFd, addr, SEEK_SET) == -1) {
    close(memFd);
    pidDetach(pid);
    mutex.unlock();
    throw MedException("Failed lseek");
  }
  if(::read(memFd, mem->getData(), size) == -1) {
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
  int writeSize = size ? size : mem->getSize();
  memcpy(ptr, mem->getData(), writeSize);
}

void MemIO::writeProcess(Address addr, MemPtr mem, size_t size) {
  mutex.lock();
  pidAttach(pid);

  int writeSize = size ? size : mem->getSize();

  int psize = padWordSize(writeSize);
  Byte* buf = new Byte[writeSize];

  long word;
  for (int i = 0; i < psize; i += sizeof(long)) {
    errno = 0;
    word = ptrace(PTRACE_PEEKDATA, pid, (Byte*)(addr) + i, NULL);

    if(errno) {
      printf("PEEKDATA error: %p, %s\n", (void*)addr, strerror(errno));
    }

    //Write word to the buffer
    memcpy((Byte*)buf + i, &word, sizeof(long));
  }

  memcpy(buf, mem->getData(), writeSize); //over-write on top of it, so that the last padding byte will preserved

  for (int i = 0; i < writeSize; i += sizeof(long)) {
    // This writes as uint32, it should be uint8
    // According to manual, it reads "word". Depend on the CPU.
    // If the OS is 32bit, then word is 32bit; if 64bit, then 64bit.
    // Thus, the best solution is peek first, then only over write the position
    // Therefore, the value should be the WORD size.

    if (ptrace(PTRACE_POKEDATA, pid, (Byte*)(addr) + i, *(long*)((Byte*)buf + i) ) == -1L) {
      printf("POKEDATA error: %s\n", strerror(errno));
    }
  }

  delete[] buf;
  pidDetach(pid);
  mutex.unlock();
}
