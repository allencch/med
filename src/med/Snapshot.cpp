#include <iostream>
#include <cstdio>

#include "med/Process.hpp"
#include "med/Snapshot.hpp"
#include "med/MedException.hpp"
#include "med/MedCommon.hpp"
#include "med/MemOperator.hpp"

using namespace std;

Snapshot::Snapshot(SnapshotScanService* service) {
  if (service == NULL) {
    this->service = new SnapshotScanService();
  }
  else {
    this->service = service;
  }
  scanUnknown = false;
}
Snapshot::~Snapshot() {
  memoryBlocks.clear();
  delete this->service;
}

void Snapshot::save(Process* process) {
  this->process = process;

  if (!hasProcess()) {
    throw MedException("Process ID is empty");
  }
  memoryBlocks.clear();
  memoryBlocks = pullProcessMemory();

  scanUnknown = true;
}

vector<SnapshotScan*> Snapshot::compare(const ScanParser::OpType& opType, const ScanType& scanType) {
  vector<SnapshotScan*> result;
  if (opType == ScanParser::OpType::SnapshotSave) {
    cerr << "Filter should not use \"?\"" << endl;
    return result;
  }

  if (!hasProcess()) {
    throw MedException("Process ID is empty");
  }


  if (isUnknown()) {
    if (memoryBlocks.getSize() == 0) {
      throw MedException("Memory blocks is empty");
    }

    MemoryBlocks currentMemoryBlocks = pullProcessMemory();

    MemoryBlockPairs pairs = createMemoryBlockPairs(memoryBlocks, currentMemoryBlocks);

    result = filterUnknown(pairs, opType, scanType);
  }
  else {
    result = filter(opType, scanType);
  }
  return result;
}

MemoryBlockPairs Snapshot::createMemoryBlockPairs(MemoryBlocks prev, MemoryBlocks curr) {
  MemoryBlockPairs pairs;
  vector<MemoryBlock> prevBlocks = prev.getData();
  vector<MemoryBlock> currBlocks = curr.getData();
  for (size_t i = 0; i < prevBlocks.size(); i++) {
    for (size_t j = 0; j < currBlocks.size(); j++) {
      if (isBlockMatched(prevBlocks[i], currBlocks[j])) {
        MemoryBlockPair pair(currBlocks[j], prevBlocks[i]); // The current one compare to the previous one.
        pairs.push_back(pair);
        break;
      }
    }
  }
  return pairs;
}

bool Snapshot::isBlockMatched(MemoryBlock block1, MemoryBlock block2) {
  MemAddr firstAddress1 = block1.getAddress();
  MemAddr lastAddress1 = firstAddress1 + block1.getSize();
  MemAddr firstAddress2 = block2.getAddress();
  MemAddr lastAddress2 = firstAddress2 + block2.getSize();
  return (firstAddress1 <= firstAddress2 && lastAddress1 >= firstAddress2) ||
    (firstAddress1 <= lastAddress2 && lastAddress1 >= lastAddress2);
}

vector<SnapshotScan*> Snapshot::filterUnknown(const MemoryBlockPairs& pairs, const ScanParser::OpType& opType, const ScanType& scanType) {
  for (size_t i = 0; i < pairs.size(); i++) {
    vector<SnapshotScan*> found = comparePair(pairs[i], opType, scanType);
    scans.insert(scans.end(), found.begin(), found.end());
  }
  clearUnknown();
  return scans;
}

vector<SnapshotScan*> Snapshot::comparePair(const MemoryBlockPair& pair, const ScanParser::OpType& opType, const ScanType& scanType) {
  MemoryBlock curr = pair.first;
  MemoryBlock prev = pair.second;

  int offset = prev.getAddress() - curr.getAddress();
  int currOffset = 0;
  int prevOffset = 0;
  if (offset < 0) {
    prevOffset = offset;
  }
  else {
    currOffset = offset;
  }

  int currLength = curr.getSize();
  int prevLength = prev.getSize();

  int typeSize = scanTypeToSize(scanType);

  auto currData = curr.getData();
  auto prevData = prev.getData();

  vector<SnapshotScan*> scan;
  for (int i = 0; (i < currLength - typeSize + 1 - currOffset) && (i < prevLength - typeSize + 1 + prevOffset); i++) {

    bool result = memCompare(&currData[i + currOffset], &prevData[i - prevOffset], typeSize, opType);
    if (result) {
      SnapshotScan* matched = new SnapshotScan(curr.getAddress() + i + currOffset, scanType);
      Bytes* copied = Bytes::newCopy(&currData[i + currOffset], typeSize);
      matched->setScannedValue(copied);
      scan.push_back(matched);
    }
  }
  return scan;
}

bool Snapshot::isUnknown() {
  return scanUnknown;
}

void Snapshot::clearUnknown() {
  scanUnknown = false;
  memoryBlocks.clear();
}

vector<SnapshotScan*> Snapshot::filter(const ScanParser::OpType& opType, const ScanType& scanType) {
  vector<SnapshotScan*> newScans;
  for (size_t i = 0; i < scans.size(); i++) {
    SnapshotScan* scan = scans[i];
    long pid = getProcessPid();
    if (service->compareScan(scan, pid, opType, scanType)) {
      // FIXME: Error free invalid pointer
      service->updateScannedValue(scan, pid, scanType);
      newScans.push_back(scan);
    }
    else {
      // FIXME: Error free invalid pointer
      scan->freeScannedValue();
      delete scan;
      scan = NULL;
    }
  }

  scans = newScans;
  return scans;
}

bool Snapshot::hasProcess() {
  return process->pid.length() != 0;
}

MemoryBlocks Snapshot::pullProcessMemory() {
  return process->pullMemory();
}

long Snapshot::getProcessPid() {
  return stol(process->pid);
}
