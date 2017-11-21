#include <cstring>

#include "med/Snapshot.hpp"

class SnapshotTester : public Snapshot {
public:
  SnapshotTester() : Snapshot() {}
  virtual ~SnapshotTester() {
    memoryBlocks.clear();
  }

  virtual bool hasProcess() {
    return true;
  }

  virtual MemoryBlocks pullProcessMemory() { // Assuming it always pull the same data
    Byte* data1 = new Byte[12];
    memset(data1, 0, 12);
    data1[0] = 40;
    MemoryBlock block1(data1, 12);
    block1.setAddress(0x08002000);

    Byte* data2 = new Byte[20];
    memset(data2, 0, 20);
    data2[0] = 50;
    MemoryBlock block2(data2, 20);
    block2.setAddress(0x08003000);

    MemoryBlocks blocks;
    blocks.push(block1);
    blocks.push(block2);
    return blocks;
  }
};

class TestSnapshot : public CxxTest::TestSuite {
public:
  void testInstantiation() {
    // should instantiate the snapshot
    SnapshotTester* snapshot = new SnapshotTester();
    delete snapshot;

    TS_ASSERT(snapshot != 0);
  }

  void testComparePairGreater() {
    Byte* data1 = new Byte[12];
    memset(data1, 0, 12);
    data1[0] = 40;
    MemoryBlock block1(data1, 12);
    block1.setAddress(0x08002000);

    Byte* data2 = new Byte[12];
    memset(data2, 0, 12);
    data2[0] = 30;
    MemoryBlock block2(data2, 12);
    block2.setAddress(0x08002000);

    MemoryBlockPair pair(block1, block2);

    SnapshotTester* snapshot = new SnapshotTester();
    vector<SnapshotScan*> output = snapshot->comparePair(pair, ScanParser::OpType::Gt, ScanType::Int32);

    TS_ASSERT_EQUALS(output.size(), 1);

    Byte* data = output[0]->getScannedValue()->getData();
    TS_ASSERT_EQUALS(data[0], 40);
    TS_ASSERT_EQUALS(output[0]->getAddress(), 0x08002000);

    delete snapshot;
  }

  void testComparePairLess() {
    Byte* data1 = new Byte[12];
    memset(data1, 0, 12);
    data1[0] = 10;
    MemoryBlock block1(data1, 12);
    block1.setAddress(0x08002000);

    Byte* data2 = new Byte[12];
    memset(data2, 0, 12);
    data2[0] = 30;
    MemoryBlock block2(data2, 12);
    block2.setAddress(0x08002000);

    MemoryBlockPair pair(block1, block2);

    SnapshotTester* snapshot = new SnapshotTester();
    vector<SnapshotScan*> output = snapshot->comparePair(pair, ScanParser::OpType::Lt, ScanType::Int32);

    TS_ASSERT_EQUALS(output.size(), 1);

    Byte* data = output[0]->getScannedValue()->getData();
    TS_ASSERT_EQUALS(data[0], 10);
    TS_ASSERT_EQUALS(output[0]->getAddress(), 0x08002000);

    delete snapshot;
  }

  void testComparePairOffsetDifference1() {
    Byte* data1 = new Byte[6]; // Current memory
    memset(data1, 0, 6);
    data1[0] = 10;
    MemoryBlock block1(data1, 6);
    block1.setAddress(0x08002001);

    Byte* data2 = new Byte[6]; // Previous memory
    memset(data2, 0, 6);
    data2[1] = 30;
    MemoryBlock block2(data2, 6);
    block2.setAddress(0x08002000);

    MemoryBlockPair pair(block1, block2);

    /*
      Curr: xx 0a 00 00 00 00 00 ...
      Prev: 00 1e 00 00 00 00 xx ...
      It should compare based on alignment.
     */

    SnapshotTester* snapshot = new SnapshotTester();
    vector<SnapshotScan*> output = snapshot->comparePair(pair, ScanParser::OpType::Lt, ScanType::Int32);

    TS_ASSERT_EQUALS(output.size(), 1);
    Byte* data = output[0]->getScannedValue()->getData();
    TS_ASSERT_EQUALS(data[0], 10);
    TS_ASSERT_EQUALS(output[0]->getAddress(), 0x08002001);

    delete snapshot;
  }

  void testComparePairOffsetDifference2() {
    Byte* data1 = new Byte[6]; // Current memory
    memset(data1, 0, 6);
    data1[0] = 10;
    MemoryBlock block1(data1, 6);
    block1.setAddress(0x08002001);

    Byte* data2 = new Byte[7]; // Previous memory
    memset(data2, 0, 7);
    data2[1] = 30;
    MemoryBlock block2(data2, 7);
    block2.setAddress(0x08002000);

    MemoryBlockPair pair(block1, block2);

    /*
      Curr: xx 0a 00 00 00 00 00 ...
      Prev: 00 1e 00 00 00 00 00 ...
     */

    SnapshotTester* snapshot = new SnapshotTester();
    vector<SnapshotScan*> output = snapshot->comparePair(pair, ScanParser::OpType::Lt, ScanType::Int32);

    TS_ASSERT_EQUALS(output.size(), 1);
    Byte* data = output[0]->getScannedValue()->getData();
    TS_ASSERT_EQUALS(data[0], 10);
    TS_ASSERT_EQUALS(output[0]->getAddress(), 0x08002001);

    delete snapshot;
  }

  void testComparePairOffsetDifference3() {
    Byte* data1 = new Byte[6]; // Current memory
    memset(data1, 0, 6);
    data1[1] = 10;
    MemoryBlock block1(data1, 6);
    block1.setAddress(0x08002000);

    Byte* data2 = new Byte[6]; // Previous memory
    memset(data2, 0, 6);
    data2[0] = 30;
    MemoryBlock block2(data2, 6);
    block2.setAddress(0x08002001);

    MemoryBlockPair pair(block1, block2);

    /*
      Curr: 00 0a 00 00 00 00 xx ...
      Prev: xx 1e 00 00 00 00 00 ...
     */

    SnapshotTester* snapshot = new SnapshotTester();
    vector<SnapshotScan*> output = snapshot->comparePair(pair, ScanParser::OpType::Lt, ScanType::Int32);

    TS_ASSERT_EQUALS(output.size(), 1);
    Byte* data = output[0]->getScannedValue()->getData();
    TS_ASSERT_EQUALS(data[0], 10);
    TS_ASSERT_EQUALS(output[0]->getAddress(), 0x08002001);

    delete snapshot;
  }

  void testComparePairOffsetDifference4() {
    Byte* data1 = new Byte[7]; // Current memory
    memset(data1, 0, 7);
    data1[1] = 10;
    MemoryBlock block1(data1, 7);
    block1.setAddress(0x08002000);

    Byte* data2 = new Byte[6]; // Previous memory
    memset(data2, 0, 6);
    data2[0] = 30;
    MemoryBlock block2(data2, 6);
    block2.setAddress(0x08002001);

    MemoryBlockPair pair(block1, block2);

    /*
      Curr: 00 0a 00 00 00 00 00 ...
      Prev: xx 1e 00 00 00 00 00 ...
     */

    SnapshotTester* snapshot = new SnapshotTester();
    vector<SnapshotScan*> output = snapshot->comparePair(pair, ScanParser::OpType::Lt, ScanType::Int32);

    TS_ASSERT_EQUALS(output.size(), 1);
    Byte* data = output[0]->getScannedValue()->getData();
    TS_ASSERT_EQUALS(data[0], 10);
    TS_ASSERT_EQUALS(output[0]->getAddress(), 0x08002001);

    delete snapshot;
  }

  void testComparePairOffsetDifference5() {
    Byte* data1 = new Byte[12]; // Current memory
    memset(data1, 0, 12);
    data1[0] = 10;
    data1[4] = 20;
    MemoryBlock block1(data1, 12);
    block1.setAddress(0x08002000);

    Byte* data2 = new Byte[12]; // Previous memory
    memset(data2, 0, 12);
    data2[0] = 30;
    data2[4] = 40;
    MemoryBlock block2(data2, 12);
    block2.setAddress(0x08002000);

    MemoryBlockPair pair(block1, block2);

    /*
      curr: 0a 00 00 00, 14 00 00 00, 00 00 00 00
      prev: 1e 00 00 00, 28 00 00 00, 00 00 00 00
      It should have 5
     */

    SnapshotTester* snapshot = new SnapshotTester();
    vector<SnapshotScan*> output = snapshot->comparePair(pair, ScanParser::OpType::Lt, ScanType::Int32);

    TS_ASSERT_EQUALS(output.size(), 5);
    Byte* scanned1 = output[0]->getScannedValue()->getData();
    TS_ASSERT_EQUALS(scanned1[0], 10);
    TS_ASSERT_EQUALS(output[0]->getAddress(), 0x08002000);

    Byte* scanned2 = output[4]->getScannedValue()->getData();
    TS_ASSERT_EQUALS(scanned2[0], 20);
    TS_ASSERT_EQUALS(output[4]->getAddress(), 0x08002004);

    delete snapshot;
  }

  void testComparePairOffsetDifference6() {
    Byte* data1 = new Byte[12]; // Current memory
    memset(data1, 0, 12);
    data1[0] = 10;
    data1[4] = 20;
    MemoryBlock block1(data1, 12);
    block1.setAddress(0x08002000);

    Byte* data2 = new Byte[8]; // Previous memory
    memset(data2, 0, 8);
    data2[0] = 30;
    data2[4] = 40;
    MemoryBlock block2(data2, 8);
    block2.setAddress(0x08002004);

    MemoryBlockPair pair(block1, block2);

    /*
      curr: 0a 00 00 00, 14 00 00 00, 00 00 00 00
      prev: xx xx xx xx, 1e 00 00 00, 28 00 00 00
      It should have 5
     */

    SnapshotTester* snapshot = new SnapshotTester();
    vector<SnapshotScan*> output = snapshot->comparePair(pair, ScanParser::OpType::Lt, ScanType::Int32);

    TS_ASSERT_EQUALS(output.size(), 5);
    Byte* scanned1 = output[0]->getScannedValue()->getData();
    TS_ASSERT_EQUALS(scanned1[0], 20);
    TS_ASSERT_EQUALS(output[0]->getAddress(), 0x08002004);

    Byte* scanned2 = output[4]->getScannedValue()->getData();
    TS_ASSERT_EQUALS(scanned2[0], 0);
    TS_ASSERT_EQUALS(output[4]->getAddress(), 0x08002008);

    delete snapshot;
  }

  void testCompare() {
    // should compare first time
    SnapshotTester* snapshot = new SnapshotTester();
    snapshot->scanUnknown = true;

    // Set the memory blocks
    Byte* data1 = new Byte[12];
    memset(data1, 0, 12);
    data1[0] = 20;
    MemoryBlock block1(data1, 12);
    block1.setAddress(0x08002000);

    Byte* data2 = new Byte[20];
    memset(data2, 0, 20);
    data2[0] = 30;
    MemoryBlock block2(data1, 20);
    block2.setAddress(0x08003000);

    MemoryBlocks blocks;
    blocks.push(block1);
    blocks.push(block2);

    snapshot->memoryBlocks = blocks;

    vector<SnapshotScan*> output = snapshot->compare(ScanParser::OpType::Gt, ScanType::Int32);

    delete snapshot;

    TS_ASSERT_EQUALS(output.size(), 2);
    SnapshotScan* scan1 = output[0];
    TS_ASSERT_EQUALS(scan1->getAddress(), 0x8002000);
    SnapshotScan* scan2 = output[1];
    TS_ASSERT_EQUALS(scan2->getAddress(), 0x8003000);

    Byte* bytes1 = scan1->getScannedValue()->getData();
    Byte* bytes2 = scan2->getScannedValue()->getData();
    TS_ASSERT_EQUALS(bytes1[0], 40);
    TS_ASSERT_EQUALS(bytes2[0], 50);
  }

  // void testFilter() {
  //   SnapshotTester* snapshot = new SnapshotTester();
  //   snapshot->scanUnknown = true;

  //   // Set the memory blocks
  //   Byte* data1 = new Byte[12];
  //   memset(data1, 0, 12);
  //   data1[0] = 20;
  //   MemoryBlock block1(data1, 12);
  //   block1.setAddress(0x08002000);

  //   Byte* data2 = new Byte[20];
  //   memset(data2, 0, 20);
  //   data2[0] = 30;
  //   MemoryBlock block2(data1, 20);
  //   block2.setAddress(0x08003000);

  //   MemoryBlocks blocks;
  //   blocks.push(block1);
  //   blocks.push(block2);

  //   snapshot->memoryBlocks = blocks;

  //   snapshot->compare(ScanParser::OpType::Gt, ScanType::Int32);

  //   vector<SnapshotScan*> output = snapshot->filter(ScanParser::OpType::Gt, ScanType::Int32);

  //   delete snapshot;

  //   TS_ASSERT_EQUALS(output.size(), 2);
  //   SnapshotScan* scan1 = output[0];
  //   TS_ASSERT_EQUALS(scan1->getAddress(), 0x8002000);
  //   SnapshotScan* scan2 = output[1];
  //   TS_ASSERT_EQUALS(scan2->getAddress(), 0x8003000);

  //   Byte* bytes1 = scan1->getScannedValue()->getData();
  //   Byte* bytes2 = scan2->getScannedValue()->getData();
  //   TS_ASSERT_EQUALS(bytes1[0], 40);
  //   TS_ASSERT_EQUALS(bytes2[0], 50);
  // }
};
