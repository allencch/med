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

void Snapshot::compare(const ScanParser::OpType& opType, const ScanType& scanType) {
  if (process->pid.length() == 0) {
    MedException("Process ID is empty");
    return;
  }
  if (memoryBlocks.getSize() == 0) {
    MedException("Memory blocks is empty");
    return;
  }
  MemoryBlocks currentMemoryBlocks = process->pullMemory();
  MemoryBlockPairs pairs = createMemoryBlockPairs(memoryBlocks, currentMemoryBlocks);
  filter(pairs, opType, scanType);
}

MemoryBlockPairs Snapshot::createMemoryBlockPairs(MemoryBlocks prev, MemoryBlocks curr) {
  MemoryBlockPairs pairs;
  vector<MemoryBlock> prevBlocks = prev.getData();
  vector<MemoryBlock> currBlocks = curr.getData();
  for (size_t i = 0; i < prevBlocks.size(); i++) {
    for (size_t j = 0; j < currBlocks.size(); j++) {
      if (blockMatched(prevBlocks[i], currBlocks[j])) {
        MemoryBlockPair pair(prevBlocks[i], currBlocks[j]);
        pairs.push_back(pair);
        break;
      }
    }
  }
  return pairs;
}

bool Snapshot::blockMatched(MemoryBlock block1, MemoryBlock block2) {
  MemAddr firstAddress1 = block1.getAddress();
  MemAddr lastAddress1 = firstAddress1 + block1.getSize();
  MemAddr firstAddress2 = block2.getAddress();
  MemAddr lastAddress2 = firstAddress2 + block2.getSize();
  return (firstAddress1 <= firstAddress2 && lastAddress1 >= firstAddress2) ||
    (firstAddress1 <= lastAddress2 && lastAddress1 >= lastAddress2);
}

void Snapshot::filter(const MemoryBlockPairs, const ScanParser::OpType& opType, const ScanType& scanType) {

}
