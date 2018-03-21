#include <cstdio>
#include <algorithm>

#include "mem/MemList.hpp"
#include "med/MedCommon.hpp"
#include "mem/Sem.hpp"

using namespace std;

MemList::MemList(vector<MemPtr> list) {
  this->list = list;
}

size_t MemList::size() {
  return list.size();
}

string MemList::getAddressAsString(int index) {
  return list[index]->getAddressAsString();
}

Address MemList::getAddress(int index) {
  return list[index]->getAddress();
}

string MemList::getValue(int index, const string& scanType) {
  PemPtr pem = static_pointer_cast<Pem>(list[index]);
  return pem->getValue(scanType);
}

string MemList::getValue(int index) {
  PemPtr pem = static_pointer_cast<Pem>(list[index]);
  return pem->getValue(pem->getScanType());
}

void MemList::dump(int index, bool newline) {
  list[index]->dump(newline);
}

string MemList::getScanType(int index) {
  PemPtr pem = static_pointer_cast<Pem>(list[index]);
  return pem->getScanType();
}

void MemList::setValue(int index, const string& value, const string& scanType) {
  PemPtr pem = static_pointer_cast<Pem>(list[index]);
  pem->setValue(value, scanType);
}

void MemList::setScanType(int index, const string& scanType) {
  PemPtr pem = static_pointer_cast<Pem>(list[index]);
  pem->setScanType(scanType);
}

int MemList::getLastIndex() {
  return list.size() - 1;
}

void MemList::sortByAddress() {
  MemList::sortByAddress(list);
}

void MemList::clear() {
  list.clear();
}

void MemList::setAddress(int index, const string& address) {
  list[index]->setAddress(hexToInt(address));
}

vector<MemPtr> MemList::sortByAddress(vector<MemPtr>& list) {
  sort(list.begin(), list.end(), [](MemPtr a, MemPtr b) {
      return a->getAddress() < b->getAddress();
    });

  return list;
}

void MemList::sortByDescription() {
  MemList::sortByDescription(list);
}

vector<MemPtr> MemList::sortByDescription(vector<MemPtr>& list) {
  sort(list.begin(), list.end(), [](MemPtr a, MemPtr b) {
      auto sem_a = static_pointer_cast<Sem>(a);
      auto sem_b = static_pointer_cast<Sem>(b);
      return sem_a->getDescription() < sem_b->getDescription();
    });

  return list;
}

MemPtr MemList::getMemPtr(int index) {
  return list[index];
}

void MemList::addMemPtr(MemPtr mem) {
  list.push_back(mem);
}

void MemList::setList(const vector<MemPtr>& list) {
  this->list = list;
}
vector<MemPtr>& MemList::getList() {
  return list;
}

void MemList::addNextAddress(int index) {
  auto mem = list[index];
  SemPtr semPtr = static_pointer_cast<Sem>(mem);
  SemPtr newSem = Sem::clone(semPtr);

  int step = scanTypeToSize(semPtr->getScanType());
  Address addr = semPtr->getAddress();
  newSem->setAddress(addr + step);
  newSem->setDescription("No description");

  list.push_back(newSem);
}

void MemList::addPrevAddress(int index) {
  auto mem = list[index];
  SemPtr semPtr = static_pointer_cast<Sem>(mem);
  SemPtr newSem = Sem::clone(semPtr);

  int step = scanTypeToSize(semPtr->getScanType());
  Address addr = semPtr->getAddress();
  newSem->setAddress(addr - step);
  newSem->setDescription("No description");

  list.push_back(newSem);
}

void MemList::shiftAddress(int index, long diff) {
  auto mem = list[index];
  Address addr = mem->getAddress();
  mem->setAddress(addr + diff);
}

void MemList::deleteAddress(int index) {
  list.erase(list.begin() + index);
}
