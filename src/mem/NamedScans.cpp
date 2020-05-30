#include <iostream>
#include "mem/NamedScans.hpp"
#include "med/MedTypes.hpp"

using namespace std;

NamedScans::NamedScans() {
  data[DEFAULT] = MemList();
  activeName = DEFAULT;
  scanType = SCAN_TYPE_INT_8;
}

MemList* NamedScans::addNewScan(string name) {
  if (!name.size()) return NULL;

  auto search = data.find(name);
  if (search != data.end()) {
    return NULL;
  }

  data[name] = MemList();
  return &data[name];
}

MemList* NamedScans::getMemList() {
  return getMemList(activeName);
}

MemList* NamedScans::getMemList(string name) {
  if (!name.size()) return NULL;

  auto search = data.find(name);
  if (search != data.end()) {
    return &search->second;
  }
  return NULL;
}

void NamedScans::setMemPtrs(vector<MemPtr> list, string scanType) {
  getMemList()->setList(list);
  setScanType(scanType);
}

bool NamedScans::remove(string name) {
  if (name == DEFAULT || !name.size()) return false;

  auto search = data.find(name);
  if (search != data.end()) {
    data.erase(search);
    activeName = DEFAULT;
    return true;
  }
  return false;
}

string NamedScans::getActiveName() {
  return activeName;
}

void NamedScans::setActiveName(string name) {
  if (!name.size()) return;
  activeName = name;
}

void NamedScans::setScanType(string type) {
  scanType = type;
}

string NamedScans::getScanType() {
  return scanType;
}
