#ifndef MEM_IO_H
#define MEM_IO_H

#include <mutex>
#include "med/MedTypes.hpp"
#include "mem/Mem.hpp"

class MemIO {
public:
  MemIO();
  void setPid(pid_t pid);
  pid_t getPid();
  MemPtr read(Address addr, size_t size);
  void write(Address addr, MemPtr mem, size_t size = 0);

private:
  MemPtr readProcess(Address addr, size_t size);
  MemPtr readDirect(Address addr, size_t size);
  void writeProcess(Address addr, MemPtr mem, size_t size);
  void writeDirect(Address addr, MemPtr mem, size_t size);
  pid_t pid;
  std::mutex mutex;
};

#endif
