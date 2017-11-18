#include <iostream>

#include "med/med.hpp"
#include "med/MedScan.hpp"
#include "med/MemOperator.hpp"
#include "med/MedCommon.hpp"

using namespace std;

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

Bytes* MedScan::getValueAsNewBytes(long pid, ScanType scanType) {
  Bytes* bytes = stringToNewBytes(getValue(pid, scanTypeToString(scanType)), scanType);
  return bytes;
}

/******************\
 SnapshotScan
\******************/
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
    scannedValue->dump();
    scannedValue->free();
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
  cout << 100 << endl;
  currentBytes->free();
  delete currentBytes;
  cout << 200 << endl;
  return result;
}

void SnapshotScan::updateScannedValue(long pid, ScanType scanType) {
  Bytes* currentBytes = getValueAsNewBytes(pid, scanType);
  currentBytes->dump();
  cout << 300 << endl;
  freeScannedValue();
  cout << 400 << endl;
  setScannedValue(currentBytes);
}
