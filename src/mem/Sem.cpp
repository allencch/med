#include <cstring>
#include "mem/Sem.hpp"

Sem::Sem(PemPtr pem) : Pem(pem->getSize(), pem->getMemIO()) {
  memcpy(data, pem->getData(), size);
  setAddress(pem->getAddress());
  setScanType(pem->getScanType());
  locked = false;
  description = "No description";
}

Sem::Sem(Sem& sem) : Pem(sem.getSize(), sem.getMemIO()) {
  memcpy(data, sem.getData(), size);
  setAddress(sem.getAddress());
  setScanType(sem.getScanType());
  locked = false;
  description = sem.getDescription();
}

Sem::Sem(size_t size, MemIO* memio) : Pem(size, memio) {
  locked = false;
}

Sem::Sem(Address addr, size_t size, MemIO* memio) : Pem(addr, size, memio) {
  locked = false;
}

bool Sem::isLocked() {
  return locked;
}

void Sem::lock(bool value) {
  if (value) {
    setLockedValue(getValue(getScanType()));
  }
  locked = value;
}

string Sem::getDescription() {
  return description;
}

void Sem::setDescription(string s) {
  description = s;
}

void Sem::setLockedValue(string s) {
  lockedValue = s;
}

string& Sem::getLockedValue() {
  return lockedValue;
}

void Sem::lockValue() {
  setValue(getLockedValue(), getScanType());
}

SemPtr Sem::clone(SemPtr semPtr) {
  // It is:
  // Sem* storedPtr = semPtr.get();
  // Sem* newSem = new Sem(*storedPtr); // copy constructor
  // SemPtr newSemPtr = SemPtr(new Sem(newSem)); // convert to SemPtr
  // Finally, push to the list.
  // Simplify it as following line
  return SemPtr(new Sem(*semPtr.get()));
}

SemPtr Sem::convertToSemPtr(PemPtr pem) {
  return SemPtr(new Sem(pem));
}
