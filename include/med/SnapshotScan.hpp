#ifndef SNAPSHOT_SCAN_HPP
#define SNAPSHOT_SCAN_HPP

#include <memory>

#include "med/Bytes.hpp"
#include "med/MedScan.hpp"

class SnapshotScan;

typedef std::shared_ptr<SnapshotScan> SnapshotScanPtr;

/**
 * This is the scanned address from the snapshot, which contains the scanned value as Bytes
 */
class SnapshotScan : public MedScan {
public:
  SnapshotScan();
  SnapshotScan(MemAddr address, ScanType scanType);
  virtual ~SnapshotScan();
  Bytes* getScannedValue();
  void setScannedValue(Bytes* bytes);
  void freeScannedValue();
  void updateScannedValue(long pid, ScanType scanType);

  bool compare(long pid, const ScanParser::OpType& opType, const ScanType& scanType);

  MedScan toMedScan();

  static vector<MedScan> toMedScans(const vector<SnapshotScanPtr>& snapshotScans);

  static void freeSnapshotScans(const vector<SnapshotScanPtr>& snapshotScans);

  Bytes* scannedValue; // TODO: need to delete
};

#endif
