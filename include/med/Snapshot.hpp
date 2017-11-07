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
  void compare(const ScanParser::OpType& opType, const ScanType& scanType);
  void filter(const MemoryBlockPairs, const ScanParser::OpType& opType, const ScanType& scanType);

  static bool blockMatched(MemoryBlock block1, MemoryBlock block2);

private:
  MemoryBlocks memoryBlocks;
  Process* process;

  MemoryBlockPairs createMemoryBlockPairs(MemoryBlocks prev, MemoryBlocks curr);
};

#endif
