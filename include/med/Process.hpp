#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <string>
#include <mutex>
#include <vector>

#include "med/MedTypes.hpp"
#include "med/Bytes.hpp"

using namespace std;

class MemoryBlock : public Bytes {
public:
  MemoryBlock();
  MemoryBlock(Byte* data, int size);

  void setDataWithAddress(Byte* data, int size, MemAddr address);
  void setAddress(MemAddr address);
  MemAddr getAddress();

private:
  MemAddr address;
};

class MemoryBlocks {
public:
  MemoryBlocks(); // Do not destruct by freeing the memory. Free on demand
  void free();
  void push(MemoryBlock block);
  void clear();
private:
  vector<MemoryBlock> data;
};

class Process {
public:
  Process();
  Process(const Process& process); // Required, because mutex is not copiable
  Process& operator=(const Process& process);

  string pid;
  string cmdline; //aka "process" in GUI

  MemoryBlocks pullMemory();
  void pullMemoryByMap(const ProcMaps& maps, int mapIndex, int memFd, MemoryBlocks& blocks);

private:
  std::mutex mtx;
};

#endif
