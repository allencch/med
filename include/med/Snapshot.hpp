#ifndef SNAPSHOT_HPP
#define SNAPSHOT_HPP

#include <string>
#include <utility>
#include <vector>

#include "med/Process.hpp"
#include "med/ScanParser.hpp"
#include "med/MedScan.hpp"

typedef pair<MemoryBlock, MemoryBlock> MemoryBlockPair;
typedef vector<MemoryBlockPair> MemoryBlockPairs;

class Snapshot {
public:
  Snapshot();
  ~Snapshot();
  void save(Process* process);
  vector<MedScan> compare(const ScanParser::OpType& opType, const ScanType& scanType);
  vector<MedScan> filterUnknown(const MemoryBlockPairs& pairs, const ScanParser::OpType& opType, const ScanType& scanType);
  vector<SnapshotScan*> comparePair(const MemoryBlockPair& pair, const ScanParser::OpType& opType, const ScanType& scanType);
  vector<MedScan> filter(const ScanParser::OpType& opType, const ScanType& scanType);

  bool isUnknown();

  static bool isBlockMatched(MemoryBlock block1, MemoryBlock block2);

private:
  bool scanUnknown;
  MemoryBlocks memoryBlocks;
  Process* process;

  vector<SnapshotScan*> scans; // TODO: Need to free SnapshotScan*

  MemoryBlockPairs createMemoryBlockPairs(MemoryBlocks prev, MemoryBlocks curr);

  void clearUnknown();
};

#endif
