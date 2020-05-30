#include <iostream>
#include "mem/NamedScans.hpp"
#include "med/MedTypes.hpp"

using namespace std;

NamedScans::NamedScans() {
  data[DEFAULT] = MemList();
  activeName = DEFAULT;
  scanTypes[DEFAULT] = SCAN_TYPE_INT_8;
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
    removeScanTypes(name);
    activeName = DEFAULT;
    return true;
  }
  return false;
}

void NamedScans::removeScanTypes(string name) {
  auto search = scanTypes.find(name);
  if (search != scanTypes.end()) {
    scanTypes.erase(search);
  }
}

string NamedScans::getActiveName() {
  return activeName;
}

void NamedScans::setActiveName(string name) {
  if (!name.size()) return;
  activeName = name;
}

void NamedScans::setScanType(string type) {
  scanTypes[activeName] = type;
}

string NamedScans::getScanType() {
  auto result = scanTypes[activeName];
  if (!result.length()) return SCAN_TYPE_INT_8;

  return result;
}
