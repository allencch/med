#include <cstring>
#include <iostream>

#include "med/Snapshot.hpp"
#include "med/ByteManager.hpp"

using namespace std;

ByteManager& bm = ByteManager::getInstance();

class SnapshotTester : public Snapshot {
public:
  SnapshotTester() : Snapshot() {
    data1 = bm.newByte(12);
    data2 = bm.newByte(20);
  }
  SnapshotTester(SnapshotScanService* service) : Snapshot(service) {
    data1 = bm.newByte(12);
    data2 = bm.newByte(20);
  }
  virtual ~SnapshotTester() {
    cout << " This is the problem " << endl;
    bm.deleteByte(data1);
    bm.deleteByte(data2);
    memoryBlocks.clear();
  }

  virtual bool hasProcess() {
    return true;
  }

  virtual MemoryBlocks pullProcessMemory() { // Assuming it always pull the same data
    memset(data1, 0, 12);
    data1[0] = 40;
    MemoryBlock block1(data1, 12);
    block1.setAddress(0x08002000);

    memset(data2, 0, 20);
    data2[0] = 50;
    MemoryBlock block2(data2, 20);
    block2.setAddress(0x08003000);

    MemoryBlocks blocks;
    blocks.push(block1);
    blocks.push(block2);
    return blocks;
  }
private:
  Byte* data1;
  Byte* data2;
};

class SnapshotScanServiceTester : public SnapshotScanService {
public:
  SnapshotScanServiceTester() {}
};


namespace TestSnapshot {
  void testCompare() {
    // should compare first time
    SnapshotTester* snapshot = new SnapshotTester();
    snapshot->scanUnknown = true;

    // Set the memory blocks
    Byte* data1 = bm.newByte(12);
    memset(data1, 0, 12);
    data1[0] = 20;
    MemoryBlock block1(data1, 12);
    block1.setAddress(0x08002000);

    Byte* data2 = bm.newByte(20);
    memset(data2, 0, 20);
    data2[0] = 30;
    MemoryBlock block2(data1, 20);
    block2.setAddress(0x08003000);

    MemoryBlocks blocks;
    blocks.push(block1);
    blocks.push(block2);

    snapshot->memoryBlocks = blocks;

    vector<SnapshotScan*> output = snapshot->compare(ScanParser::OpType::Gt, ScanType::Int32);

    bm.deleteByte(data1);
    bm.deleteByte(data2);

    delete snapshot;
  }
}

int main() {
  cout << "Test Snapshot" << endl;
  TestSnapshot::testCompare();
  return 0;
}
