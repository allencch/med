#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <string>
#include <mutex>
#include <vector>

#include "med/MedTypes.hpp"
#include "med/Bytes.hpp"

using namespace std;

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
