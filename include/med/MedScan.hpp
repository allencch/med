#ifndef MED_SCAN_H
#define MED_SCAN_H

#include <string>

#include "med/MedTypes.hpp"
#include "med/Bytes.hpp"
#include "med/ScanParser.hpp"

using namespace std;

/**
 * This is the Scanner replacement
 */
class MedScan {
public:
  MedScan();
  MedScan(MemAddr address);
  MemAddr address;

  void setAddress(MemAddr address);
  MemAddr getAddress();

  string getScanType();
  void setScanType(string s);
  ScanType scanType; //direct access

  //No value, because value is calculated based on scan type
  string getValue(long pid);
  string getValue(long pid, string scanType);
  void setValue(long pid, string val);

  Bytes getValueAsBytes(long pid, ScanType scanType);
};

/**
 * This is the scanned address from the snapshot, which contains the scanned value as Bytes
 */
class SnapshotScan : public MedScan {
public:
  SnapshotScan();
  SnapshotScan(MemAddr address, ScanType scanType);
  void setScannedValue(Bytes bytes);
  void freeScannedValue();
  void updateScannedValue(long pid, ScanType scanType);

  bool compare(long pid, const ScanParser::OpType& opType, const ScanType& scanType);

  Bytes scannedValue;

  MedScan toMedScan();

  static vector<MedScan> toMedScans(const vector<SnapshotScan*>& snapshotScans);
};

#endif
