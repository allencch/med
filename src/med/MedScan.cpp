#include "med/med.hpp"
#include "med/MedScan.hpp"
#include "med/MemOperator.hpp"
#include "med/MedCommon.hpp"


/*******************
Object (though not oriented) solution
*****************/
MedScan::MedScan() {}
MedScan::MedScan(MemAddr address) {
  this->address = address;
}
string MedScan::getScanType() {
  return scanTypeToString(this->scanType);
}

void MedScan::setScanType(string s) {
  this->scanType = stringToScanType(s);
}

string MedScan::getValue(long pid) {
  string value;

  value = memValue(pid, this->address, this->getScanType());
  return value;
}

string MedScan::getValue(long pid, string scanType) {
  string value;
  value = memValue(pid, this->address, scanType);
  return value;
}

void MedScan::setValue(long pid, string v) {
  uint8_t* buffer;
  int size = stringToRaw(v, this->scanType, &buffer);
  memWrite(pid, this->address, buffer, size);
  delete[] buffer;
}

void MedScan::setAddress(MemAddr address) {
  this->address = address;
}

MemAddr MedScan::getAddress() {
  return address;
}


/******************\
 SnapshotScan
\******************/
SnapshotScan::SnapshotScan() {}

SnapshotScan::SnapshotScan(MemAddr address) : MedScan(address) {}

void SnapshotScan::setScannedValue(Bytes bytes) {
  scannedValue = bytes;
}

void SnapshotScan::freeScannedValue() {
  scannedValue.free();
}

MedScan SnapshotScan::toMedScan() {
  MedScan medScan;
  medScan.setScanType(this->getScanType());
  medScan.setAddress(this->getAddress());
  return medScan;
}

vector<MedScan> SnapshotScan::toMedScans(const vector<SnapshotScan>& snapshotScans) {
  vector<MedScan> scans;
  for (auto snapshotScan : snapshotScans) {
    scans.push_back(snapshotScan.toMedScan());
  }
  return scans;
}
