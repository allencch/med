#include "med/SnapshotScan.hpp"
#include "med/MemOperator.hpp"

SnapshotScan::SnapshotScan() {
  scannedValue = NULL;
}

SnapshotScan::SnapshotScan(MemAddr address, ScanType scanType) : MedScan(address) {
  this->scanType = scanType;
  scannedValue = NULL;
}

void SnapshotScan::setScannedValue(Bytes* bytes) {
  scannedValue = bytes;
}

void SnapshotScan::freeScannedValue() {
  if (scannedValue) {
    scannedValue->free();
    delete scannedValue;
    scannedValue = NULL;
  }
}

MedScan SnapshotScan::toMedScan() {
  MedScan medScan;
  medScan.setScanType(this->getScanType());
  medScan.setAddress(this->getAddress());
  return medScan;
}

vector<MedScan> SnapshotScan::toMedScans(const vector<SnapshotScan*>& snapshotScans) {
  vector<MedScan> scans;
  for (auto snapshotScan : snapshotScans) {
    scans.push_back(snapshotScan->toMedScan());
  }
  return scans;
}

bool SnapshotScan::compare(long pid, const ScanParser::OpType& opType, const ScanType& scanType) {
  Bytes* currentBytes = getValueAsNewBytes(pid, scanType);
  Byte* currentData = currentBytes->getData();
  Byte* previousData = scannedValue->getData();

  bool result = memCompare(previousData, currentData, scanTypeToSize(scanType), opType);
  currentBytes->free();
  delete currentBytes;
  return result;
}

void SnapshotScan::updateScannedValue(long pid, ScanType scanType) {
  Bytes* currentBytes = getValueAsNewBytes(pid, scanType);
  freeScannedValue();
  setScannedValue(currentBytes);
}
