#ifndef SNAPSHOT_SCAN_HPP
#define SNAPSHOT_SCAN_HPP

#include "med/Bytes.hpp"
#include "med/MedScan.hpp"

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

  static vector<MedScan> toMedScans(const vector<SnapshotScan*>& snapshotScans);

  static void freeSnapshotScans(const vector<SnapshotScan*>& snapshotScans);

  Bytes* scannedValue; // TODO: need to delete
};

#endif
