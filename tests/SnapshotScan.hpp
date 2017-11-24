#include "med/SnapshotScan.hpp"
#include "med/ByteManager.hpp"

ByteManager& bm = ByteManager::getInstance();

class SnapshotScanTester : public SnapshotScan {
public:
  SnapshotScanTester() : SnapshotScan() {
    scannedValue = NULL;
  }
  virtual Bytes* getValueAsNewBytes(long pid, ScanType scanType) {
    Byte* data = bm.newByte(8);
    memset(data, 0, 8);
    data[0] = 20;
    Bytes* bytes = new Bytes(data, 8);
    return bytes;
  }
};

class TestSnapshotScan : public CxxTest::TestSuite {
public:
  void testCompare() {
    SnapshotScanTester* snapshotScan = new SnapshotScanTester();

    Byte* data = bm.newByte(8);
    memset(data, 0, 8);
    data[0] = 10;
    Bytes* bytes = new Bytes(data, 8);

    snapshotScan->scannedValue = bytes;

    bool result = snapshotScan->compare(0, ScanParser::OpType::Gt, ScanType::Int32);
    delete snapshotScan;

    TS_ASSERT_EQUALS(result, true);
  }

  void testCompareThenUpdateScannedValue() {
    SnapshotScanTester* snapshotScan = new SnapshotScanTester();

    Byte* data = bm.newByte(8);
    memset(data, 0, 8);
    data[0] = 10;
    Bytes* bytes = new Bytes(data, 8);

    snapshotScan->scannedValue = bytes;

    bool result = snapshotScan->compare(0, ScanParser::OpType::Gt, ScanType::Int32);
    snapshotScan->updateScannedValue(0, ScanType::Int32);

    TS_ASSERT_EQUALS(result, true);
    Byte* updated = snapshotScan->getScannedValue()->getData();
    TS_ASSERT_EQUALS(updated[0], 20);

    delete snapshotScan;
  }

  void testCompareThenFreeScannedValue() {
    SnapshotScanTester* snapshotScan = new SnapshotScanTester();

    Byte* data = bm.newByte(8);
    memset(data, 0, 8);
    data[0] = 30;
    Bytes* bytes = new Bytes(data, 8);

    snapshotScan->scannedValue = bytes;

    bool result = snapshotScan->compare(0, ScanParser::OpType::Gt, ScanType::Int32);
    snapshotScan->freeScannedValue();

    TS_ASSERT_EQUALS(result, false);
    delete snapshotScan;
  }
};
