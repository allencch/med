#ifndef SNAPSHOT_HPP
#define SNAPSHOT_HPP

#include <string>
#include <utility>
#include <vector>

#include "med/Process.hpp"
#include "med/ScanParser.hpp"

typedef pair<MemoryBlock, MemoryBlock> MemoryBlockPair;
typedef vector<MemoryBlockPair> MemoryBlockPairs;

class Snapshot {
public:
  Snapshot();
  ~Snapshot();
  void save(Process* process);
  void compare(const ScanParser::OpType& opType);

private:
  MemoryBlocks memoryBlocks;
  Process* process;

  MemoryBlockPairs createMemoryBlockPairs(MemoryBlocks prev, MemoryBlocks curr);
};

#endif
