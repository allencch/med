#include <iostream>

#include "med/med.hpp"
#include "med/MedScan.hpp"
#include "med/MemOperator.hpp"
#include "med/MedCommon.hpp"
#include "med/ByteManager.hpp"

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
  Byte* buffer;
  int size = stringToRaw(v, this->scanType, &buffer);
  memWrite(pid, this->address, buffer, size);

  ByteManager& bm = ByteManager::getInstance();
  bm.deleteByte(buffer);
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
