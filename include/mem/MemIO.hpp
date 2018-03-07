#ifndef MEM_IO_H
#define MEM_IO_H

#include "med/MedTypes.hpp"
#include "mem/Mem.hpp"

using namespace std;

class MemIO {
public:
  MemIO();
  void setPid(pid_t);
  pid_t getPid();
  MemPtr read(Address addr, size_t size);

private:
  pid_t pid;
};

#endif
