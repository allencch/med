#ifndef SNAPSHOT_HPP
#define SNAPSHOT_HPP

#include <string>
#include <utility>
#include <vector>

#include "med/Process.hpp"
#include "med/ScanParser.hpp"
#include "med/MedScan.hpp"
#include "med/SnapshotScan.hpp"
#include "med/SnapshotScanService.hpp"

typedef pair<MemoryBlock, MemoryBlock> MemoryBlockPair;
typedef vector<MemoryBlockPair> MemoryBlockPairs;

class Snapshot {
public:
  Snapshot(SnapshotScanService* service = NULL);
  virtual ~Snapshot();
  void save(Process* process);
  vector<SnapshotScan*> compare(const ScanParser::OpType& opType, const ScanType& scanType);
  vector<SnapshotScan*> filterUnknown(const MemoryBlockPairs& pairs, const ScanParser::OpType& opType, const ScanType& scanType);
  vector<SnapshotScan*> comparePair(const MemoryBlockPair& pair, const ScanParser::OpType& opType, const ScanType& scanType);
  vector<SnapshotScan*> filter(const ScanParser::OpType& opType, const ScanType& scanType);

  bool isUnknown();
  virtual bool hasProcess();
  virtual MemoryBlocks pullProcessMemory();

  static bool isBlockMatched(MemoryBlock block1, MemoryBlock block2);

  virtual long getProcessPid();

  bool scanUnknown;
  MemoryBlocks memoryBlocks;

private:
  Process* process;

  vector<SnapshotScan*> scans; // TODO: Need to free SnapshotScan*
  MemoryBlockPairs createMemoryBlockPairs(MemoryBlocks prev, MemoryBlocks curr);
  void clearUnknown();

  SnapshotScanService* service;
};

#endif
