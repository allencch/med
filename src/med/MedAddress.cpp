#include "med/med.hpp"
#include "med/MedAddress.hpp"
#include "med/MedCommon.hpp"

MedAddress::MedAddress() {
  this->lock = false;
  this->lockThread = NULL;
}
MedAddress::MedAddress(MemAddr address) : MedScan(address) {
  this->lock = false;
  this->lockThread = NULL;
}
MedAddress::~MedAddress() {
  if (this->lockThread) {
    this->unlockValue();
  }
}

void MedAddress::lockValue(string pid) {
  this->lock = true;
  this->lockedValue = this->getValue(stol(pid), this->getScanType());
  this->lockThread = new std::thread(::lockValue, pid, this);
}

void MedAddress::unlockValue() {
  this->lock = false;
  this->lockThread->join();
  delete this->lockThread;
  this->lockThread = NULL;
}

void MedAddress::setLockedValue(string value) {
  lockedValue = value;
}
string MedAddress::getLockedValue() {
  return lockedValue;
}
