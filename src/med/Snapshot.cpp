#include <iostream>

#include "med/Process.hpp"
#include "med/Snapshot.hpp"
#include "med/MedException.hpp"

using namespace std;

Snapshot::Snapshot() {}
Snapshot::~Snapshot() {
  memoryBlocks.clear();
}

void Snapshot::save(Process* process) {
  this->process = process;

  if (process->pid.length() == 0) {
    MedException("Process ID is empty");
    return;
  }
  memoryBlocks.clear();
  memoryBlocks = process->pullMemory();
}

void Snapshot::compare(const ScanParser::OpType& opType) {
  if (process->pid.length() == 0) {
    MedException("Process ID is empty");
    return;
  }
  if (memoryBlocks.getSize() == 0) {
    MedException("Memory blocks is empty");
    return;
  }
  MemoryBlocks currentMemoryBlocks = process->pullMemory();
  MemoryBlockPairs pais = createMemoryBlockPairs(memoryBlocks, currentMemoryBlocks);
}

MemoryBlockPairs Snapshot::createMemoryBlockPairs(MemoryBlocks prev, MemoryBlocks curr) {
  MemoryBlockPairs pairs;
  vector<MemoryBlock> prevBlocks = prev.getData();
  vector<MemoryBlock> currBlocks = curr.getData();
  for (size_t i = 0; i < prevBlocks.size(); i++) {
    for (size_t j = 0; j < currBlocks.size(); j++) {

    }
  }
   return pairs;
}
