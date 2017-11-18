#include <iostream>
#include <cstdio>

#include "med/Process.hpp"
#include "med/Snapshot.hpp"
#include "med/MedException.hpp"
#include "med/MedCommon.hpp"
#include "med/MemOperator.hpp"

using namespace std;

Snapshot::Snapshot() {
  scanUnknown = false;
}
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

  scanUnknown = true;
}

vector<MedScan> Snapshot::compare(const ScanParser::OpType& opType, const ScanType& scanType) {
  vector<MedScan> emptyScan;
  if (opType == ScanParser::OpType::SnapshotSave) {
    cerr << "Filter should not use \"?\"" << endl;
    return emptyScan;
  }

  if (process->pid.length() == 0) {
    throw MedException("Process ID is empty");
    return emptyScan;
  }

  vector<MedScan> result;
  if (isUnknown()) {
    if (memoryBlocks.getSize() == 0) {
      throw MedException("Memory blocks is empty");
      return emptyScan;
    }

    MemoryBlocks currentMemoryBlocks = process->pullMemory();
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
        MemoryBlockPair pair(prevBlocks[i], currBlocks[j]);
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

vector<MedScan> Snapshot::filterUnknown(const MemoryBlockPairs& pairs, const ScanParser::OpType& opType, const ScanType& scanType) {
  for (size_t i = 0; i < pairs.size(); i++) {
    vector<SnapshotScan*> found = comparePair(pairs[i], opType, scanType);
    scans.insert(scans.end(), found.begin(), found.end());
  }
  clearUnknown();
  return SnapshotScan::toMedScans(scans);
}


vector<SnapshotScan*> Snapshot::comparePair(const MemoryBlockPair& pair, const ScanParser::OpType& opType, const ScanType& scanType) {
  auto first = pair.first;
  auto second = pair.second;
  int offset = first.getAddress() - second.getAddress();
  int length = first.getSize();

  int typeSize = scanTypeToSize(scanType);

  vector<SnapshotScan*> scan;
  for (int i = 0; i < length; i++) {
    auto firstData = first.getData();
    auto secondData = second.getData();
    bool result = memCompare(&firstData[i], &secondData[offset + i], typeSize, opType);
    if (result) {
      SnapshotScan* matched = new SnapshotScan(first.getAddress() + i, scanType);
      Bytes copied = Bytes::copy(&secondData[offset + i], typeSize);
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

vector<MedScan> Snapshot::filter(const ScanParser::OpType& opType, const ScanType& scanType) {
  vector<SnapshotScan*> newScans;
  cout << "scan size: " << scans.size() << endl;
  for (size_t i = 0; i < scans.size(); i++) {
    auto scan = scans[i];
    long pid = stol(process->pid);
    if (scan->compare(pid, opType, scanType)) {
      scan->updateScannedValue(pid, scanType);
      newScans.push_back(scan);
    }
    else {
      cout << 500 << endl;
      scan->freeScannedValue();
      cout << 600 << endl;
    }
    printf("%d, %d\n", (int)i, (int)scans.size());
  }

  scans = newScans;
  return SnapshotScan::toMedScans(scans);
}
