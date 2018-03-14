#include <cstring>
#include "mem/Sem.hpp"

Sem::Sem(PemPtr pem) : Pem(pem->getSize(), pem->getMemIO()) {
  memcpy(data, pem->getData(), size);
  setAddress(pem->getAddress());
  setScanType(pem->getScanType());
  getValue(pem->getScanType());
  locked = false;
  description = "No description";
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
