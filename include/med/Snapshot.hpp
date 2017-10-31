#ifndef SNAPSHOT_HPP
#define SNAPSHOT_HPP

#include <string>
#include "med/Process.hpp"

class Process;

class Snapshot {
public:
  Snapshot();
  ~Snapshot();
  void save(Process* process);
private:
  MemoryBlocks memoryBlocks;
};

#endif
